from flask import Flask, request, jsonify, render_template
import sqlite3

app = Flask(__name__)

# Configuração inicial do banco de dados
def init_db():
    conn = sqlite3.connect('produtos.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS produtos (
            id TEXT PRIMARY KEY,
            nome TEXT NOT NULL
        )
    ''')
    conn.commit()
    conn.close()

# Página inicial do site
@app.route('/')
def home():
    return render_template('index.html')  # Um HTML simples que vamos criar

# Endpoint para buscar um produto
@app.route('/produto/<id>', methods=['GET'])
def buscar_produto(id):
    conn = sqlite3.connect('produtos.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM produtos WHERE id = ?', (id,))
    produto = cursor.fetchone()
    conn.close()

    if produto:
        return jsonify({'id': produto[0], 'nome': produto[1]})
    else:
        return jsonify({'error': 'Produto não encontrado'}), 404

# Endpoint para adicionar um produto via POST
@app.route('/produto', methods=['POST'])
def adicionar_produto():
    try:
        # Força o Flask a processar o corpo como JSON
        data = request.get_json(force=True)
    except Exception as e:
        return jsonify({"error": f"Erro ao processar JSON: {str(e)}"}), 400

    id_produto = data.get('id')
    nome_produto = data.get('nome')

    if not id_produto or not nome_produto:
        return jsonify({"error": "ID e Nome do produto são obrigatórios"}), 400

    conn = sqlite3.connect('produtos.db')
    cursor = conn.cursor()
    try:
        cursor.execute('INSERT INTO produtos (id, nome) VALUES (?, ?)', (id_produto, nome_produto))
        conn.commit()
    except sqlite3.IntegrityError:
        return jsonify({"error": "Produto com este ID já existe"}), 400
    finally:
        conn.close()

    return jsonify({"message": "Produto adicionado com sucesso!"})

# Endpoint para listar todos os produtos
@app.route('/produtos', methods=['GET'])
def listar_produtos():
    conn = sqlite3.connect('produtos.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM produtos')
    produtos = cursor.fetchall()
    conn.close()

    resultado = [{'id': produto[0], 'nome': produto[1]} for produto in produtos]
    return jsonify(resultado)

if __name__ == '__main__':
    init_db()  # Inicializa o banco de dados
    app.run(host='0.0.0.0', port=5000, debug=True)
