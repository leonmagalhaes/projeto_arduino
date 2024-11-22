# Use uma imagem base com Python
FROM python:3.9-slim

# Configure o diretório de trabalho no container
WORKDIR /app

# Copie os arquivos do projeto para o container
COPY . .

# Instale as dependências do projeto
RUN pip install flask
RUN touch produtos.db

# Exponha a porta usada pelo Flask
EXPOSE 5000

# Comando para iniciar o servidor
CMD ["python", "script.py"]
