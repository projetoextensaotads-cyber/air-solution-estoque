# air-solution-estoque
# Air Solution - Sistema de Almoxarifado

Programa em C++ (terminal) para controle de estoque: cadastro de materiais,
entradas, saídas para obra, ajustes e consultas de saldo. Os dados ficam em
arquivos CSV.

## Arquivos

- `main.cpp`: código-fonte
- `materiais.csv`: cadastro e saldo atual (`codigo;descricao;unidade;saldo`)
- `movimentacoes.csv`: histórico (`data hora;tipo;codigo;quantidade;saldo_anterior;saldo_novo;observacao`)

## Como compilar e executar

```
g++ -std=c++11 -Wall main.cpp -o almoxarifado
./almoxarifado
```

No Windows, execute `almoxarifado.exe`. Rode o programa **na mesma pasta dos CSVs**,
pois ele os lê pelo caminho relativo. Se não existirem, são criados na primeira gravação.

## Menus

1. **Almoxarifado**: registrar entrada, registrar saída para obra, ajuste de estoque, cadastrar material
2. **Acompanhamento de obras**: materiais enviados por obra
3. **Gestão do estoque**: saldo de todos os materiais, pesquisar material

## Regras

- Campos de texto não podem conter `;` (separador do CSV).
- Unidades aceitas: `UN` ou `M`.
- Data `DD/MM/AAAA` e hora `HH:MM`.
- Nenhuma operação altera o estoque antes da confirmação (S/N).
