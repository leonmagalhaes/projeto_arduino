#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include <SPI.h>

USB Usb;
HIDUniversal Hid(&Usb);

// Configuração WiFi
const char* ssid = "2001";       // Substitua pelo nome da sua rede WiFi
const char* password = "M100200300";   // Substitua pela senha da sua rede WiFi

// Configuração do servidor
const char* server = "192.168.15.7";  // Substitua pelo IP do servidor Flask
const int port = 5000;               // Porta do backend Flask

WiFiClient wifiClient;
HttpClient http(wifiClient, server, port);

// Buffer para armazenar o código de barras lido
String barcodeBuffer = "";

// Função para limpar o JSON
String cleanJson(String json) {
    json.trim();            // Remove espaços no início e no fim
    json.replace("\n", ""); // Remove quebras de linha
    json.replace("\r", ""); // Remove retornos de carro
    json.replace(" ", "");  // Remove espaços extras
    return json;
}

// Função para extrair valores de campos JSON
String extractJsonValue(String json, String key) {
    int keyIndex = json.indexOf("\"" + key + "\":\"");
    if (keyIndex == -1) {
        Serial.println("Erro: Chave '" + key + "' não encontrada no JSON!");
        return "";
    }

    int start = keyIndex + key.length() + 4; // Avança para o valor após "chave":""
    int end = json.indexOf("\"", start);
    if (end == -1) {
        Serial.println("Erro: Não foi possível localizar o fim do valor para a chave '" + key + "'");
        return "";
    }

    return json.substring(start, end); // Extrai o valor entre os índices
}

// Classe personalizada para processar relatórios HID
class CustomHIDReportParser : public HIDReportParser {
public:
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
        if (len > 2 && buf[2] != 0) {
            char c = KeyCodeToAscii(buf[2]);
            if (c != '\0') {
                if (c == '\n') { // Enter indica final do código
                    Serial.print("Código lido: ");
                    Serial.println(barcodeBuffer);
                    processBarcode(barcodeBuffer);
                    barcodeBuffer = ""; // Limpa o buffer
                } else {
                    barcodeBuffer += c; // Adiciona ao buffer
                }
            }
        }
    }

    char KeyCodeToAscii(uint8_t keycode) {
        switch (keycode) {
            case 0x1E: return '1';
            case 0x1F: return '2';
            case 0x20: return '3';
            case 0x21: return '4';
            case 0x22: return '5';
            case 0x23: return '6';
            case 0x24: return '7';
            case 0x25: return '8';
            case 0x26: return '9';
            case 0x27: return '0';
            case 0x28: return '\n';  // Enter
            default: return '\0';
        }
    }

    void processBarcode(String barcode) {
        if (WiFi.status() == WL_CONNECTED) {
            String getPath = "/produto/" + barcode;

            http.get(getPath);
            int statusCode = http.responseStatusCode();
            String response = http.responseBody();

            if (statusCode == 200) {
                Serial.println("Produto encontrado:");
                //Serial.println("JSON recebido:");
                //Serial.println(response); // Loga o JSON completo para depuração

                // Limpa o JSON para remover espaços e quebras de linha
                response = cleanJson(response);

                // Extrai os campos id e nome do JSON retornado
                String id = extractJsonValue(response, "id");
                String nome = extractJsonValue(response, "nome");

                // Exibe os valores formatados
                if (id.length() > 0 && nome.length() > 0) {
                    Serial.println("id: " + id);
                    Serial.println("Produto: " + nome);
                } else {
                    Serial.println("Erro: Não foi possível extrair os dados do JSON.");
                }
            } else if (statusCode == 404) {
                Serial.println("Produto não encontrado. Digite o nome do produto:");
                while (Serial.available() == 0) {
                    delay(100);
                }
                String nomeProduto = Serial.readStringUntil('\n');
                nomeProduto.trim();
                adicionarProduto(barcode, nomeProduto);
            } else {
                Serial.print("Erro na requisição GET: ");
                Serial.println(statusCode);
            }

            http.stop();
        } else {
            Serial.println("Erro: WiFi desconectado!");
        }
    }

void adicionarProduto(String barcode, String nome) {
    if (WiFi.status() == WL_CONNECTED) {
        String postPath = "/produto";
        String payload = "{\"id\":\"" + barcode + "\",\"nome\":\"" + nome + "\"}";

        Serial.println("Enviando POST com payload:");
        Serial.println(payload);

        http.beginRequest();
        http.post(postPath);
        http.sendHeader("Content-Type", "application/json");
        http.sendHeader("Content-Length", payload.length());
        http.endRequest();
        http.write((const byte*)payload.c_str(), payload.length()); // Envia o corpo do JSON

        int statusCode = http.responseStatusCode();
        String response = http.responseBody();

        Serial.print("Status Code: ");
        Serial.println(statusCode);

        if (statusCode == 200) {
            Serial.println("Produto adicionado com sucesso!");
        } else {
            Serial.println("Erro ao adicionar produto:");
            Serial.println(response);
        }

        http.stop();
    } else {
        Serial.println("Erro: WiFi desconectado!");
    }
}

};

CustomHIDReportParser ReportParser;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Iniciando USB...");

    if (Usb.Init() == -1) {
        Serial.println("Erro ao iniciar USB Host Shield");
        while (1);
    }
    Serial.println("USB Host Shield iniciado");

    Hid.SetReportParser(0, &ReportParser);

    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");
    Serial.print("IP do Arduino: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    Usb.Task();
}
