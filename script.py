from flask import Flask, request, jsonify, render_template
import sqlite3

app = Flask(__name__)

# Configuração inicial do banco de dados
def init_db():
    conn = sqlite3.connect('/app/produtos.db')
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
    
@app.route('/produtos', methods=['GET'])
def listar_produtos():
    conn = sqlite3.connect('/app/produtos.db')  # Certifique-se de ajustar o caminho do banco
    cursor = conn.cursor()

    # Recupera todos os produtos do banco
    cursor.execute('SELECT * FROM produtos')
    produtos = cursor.fetchall()
    conn.close()

    # Transforma os dados em uma lista de dicionários
    resultado = [{'id': produto[0], 'nome': produto[1]} for produto in produtos]

    return jsonify(resultado)

# Endpoint para adicionar um produto
@app.route('/produto', methods=['POST'])
def adicionar_produto():
    data = request.get_json()

    if 'id' not in data or 'nome' not in data:
        return jsonify({'error': 'ID e nome do produto são obrigatórios'}), 400

    conn = sqlite3.connect('produtos.db')
    cursor = conn.cursor()

    try:
        cursor.execute('INSERT INTO produtos (id, nome) VALUES (?, ?)', (data['id'], data['nome']))
        conn.commit()
    except sqlite3.IntegrityError:
        return jsonify({'error': 'Produto com este ID já existe'}), 400
    finally:
        conn.close()

    return jsonify({'message': 'Produto adicionado com sucesso!'})

if __name__ == '__main__':
    init_db()  # Inicializa o banco de dados
    app.run(host='0.0.0.0', port=5000, debug=True)
