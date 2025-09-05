# Basic Usage Example

Este exemplo demonstra o uso básico da biblioteca dotenv-cpp.

## Arquivos

- `main.cpp` - Exemplo principal mostrando load, get, set
- `example.env` - Arquivo de exemplo com configurações
- `CMakeLists.txt` - Configuração CMake para o exemplo

## Como executar

```bash
# A partir do diretório examples/basic_usage
mkdir build && cd build
cmake ..
make
./basic_usage_example
```

## Funcionalidades demonstradas

1. Carregamento de arquivo .env
2. Leitura de variáveis com valores padrão
3. Parsing de tipos numéricos
4. Verificação de existência de chaves
5. Definição de novas variáveis
