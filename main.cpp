
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

// ==========================================
// ESTRUTURAS DE DADOS
// ==========================================

struct Material {
    string codigo;
    string descricao;
    string unidade; // UN ou M
    double saldo;
};

struct ItemSaida {
    string codigo;
    double quantidade;
};

// ==========================================
// DECLARAÇÃO DE FUNÇÕES (PROTÓTIPOS)
// ==========================================

// Funções Utilitárias e de Validação
void pausar();
string paraMinusculo(string str);
bool contemPontoEVirgula(const string& str);
bool validarData(const string& data);
bool validarHora(const string& hora);
string lerEntradaSemSemicolon(const string& mensagem);

// Manipulação de Arquivos
vector<Material> carregarMateriais();
void salvarMateriais(const vector<Material>& materiais);
void registrarMovimentacao(const string& linhaLog);
int buscarMaterialPorCodigo(const vector<Material>& materiais, const string& codigo);

// Módulo 1: Almoxarifado
void menuAlmoxarifado();
void registrarEntrada();
void registrarSaidaObra();
void ajusteEstoque();
void cadastrarMaterial();

// Módulo 2: Acompanhamento de Obras
void menuAcompanhamentoObras();
void materiaisEnviadosPorObra();

// Módulo 3: Gestão do Estoque
void menuGestaoEstoque();
void saldoTodosMateriais();
void pesquisarMaterial();

// ==========================================
// FUNÇÃO PRINCIPAL (MAIN)
// ==========================================

int main() {
    int opcao = -1;

    // Loop do Menu Principal (Fluxograma 0)
    while (opcao != 0) {
        cout << "\n==========================================\n";
        cout << "         AIR SOLUTION MENU PRINCIPAL      \n";
        cout << "==========================================\n";
        cout << "1 - Almoxarifado\n";
        cout << "2 - Acompanhamento de obras\n";
        cout << "3 - Gestao do estoque\n";
        cout << "0 - Sair\n";
        cout << "Escolha uma opcao: ";
        
        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida. Digite um numero do menu.\n";
            continue;
        }

        switch (opcao) {
            case 1:
                menuAlmoxarifado();
                break;
            case 2:
                menuAcompanhamentoObras();
                break;
            case 3:
                menuGestaoEstoque();
                break;
            case 0:
                cout << "\nAir Solution - Menu principal\n";
                cout << "Sistema encerrado. Ate logo!\n";
                break;
            default:
                cout << "Opcao invalida. Digite um numero do menu.\n";
                break;
        }
    }

    return 0;
}

// ==========================================
// FUNÇÕES UTILITÁRIAS E VALIDAÇÃO
// ==========================================

void pausar() {
    cout << "\nPressione ENTER para continuar...";
    cin.ignore(10000, '\n');
    cin.get();
}

string paraMinusculo(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool contemPontoEVirgula(const string& str) {
    return str.find(';') != string::npos;
}

bool validarData(const string& data) {
    if (data.length() != 10) return false;
    if (data[2] != '/' || data[5] != '/') return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(data[i])) return false;
    }
    return true;
}

bool validarHora(const string& hora) {
    if (hora.length() != 5) return false;
    if (hora[2] != ':') return false;
    if (!isdigit(hora[0]) || !isdigit(hora[1]) || !isdigit(hora[3]) || !isdigit(hora[4])) return false;
    int h = stoi(hora.substr(0, 2));
    int m = stoi(hora.substr(3, 2));
    return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
}

string lerEntradaSemSemicolon(const string& mensagem) {
    string entrada;
    while (true) {
        cout << mensagem;
        cin >> entrada;
        if (contemPontoEVirgula(entrada)) {
            cout << "O campo nao pode conter ponto e virgula.\n"; // RNF de sanitização para CSV
        } else {
            return entrada;
        }
    }
}

// ==========================================
// MANIPULAÇÃO DE ARQUIVOS (CSV)
// ==========================================

// Leitura de materiais.csv (codigo;descricao;unidade;saldo)
vector<Material> carregarMateriais() {
    vector<Material> materiais;
    ifstream arquivo("materiais.csv");
    if (!arquivo.is_open()) return materiais;

    string linha;
    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;
        stringstream ss(linha);
        string cod, desc, uni, salStr;

        if (getline(ss, cod, ';') && getline(ss, desc, ';') && getline(ss, uni, ';') && getline(ss, salStr, ';')) {
            Material m;
            m.codigo = cod;
            m.descricao = desc;
            m.unidade = uni;
            m.saldo = stod(salStr);
            materiais.push_back(m);
        }
    }
    arquivo.close();
    return materiais;
}

// Gravação/Reescrita integral de materiais.csv
void salvarMateriais(const vector<Material>& materiais) {
    ofstream arquivo("materiais.csv", ios::trunc);
    for (const auto& m : materiais) {
        arquivo << m.codigo << ";" << m.descricao << ";" << m.unidade << ";" << fixed << setprecision(2) << m.saldo << "\n";
    }
    arquivo.close();
}

// Adiciona um registro ao final de movimentacoes.csv
void registrarMovimentacao(const string& linhaLog) {
    ofstream arquivo("movimentacoes.csv", ios::app);
    if (arquivo.is_open()) {
        arquivo << linhaLog << "\n";
        arquivo.close();
    }
}

int buscarMaterialPorCodigo(const vector<Material>& materiais, const string& codigo) {
    for (size_t i = 0; i < materiais.size(); ++i) {
        if (materiais[i].codigo == codigo) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ==========================================
// MÓDULO 1: ALMOXARIFADO
// ==========================================

void menuAlmoxarifado() {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n------------------------------------------\n";
        cout << "       AIR SOLUTION ALMOXARIFADO          \n";
        cout << "------------------------------------------\n";
        cout << "1 - Registrar entrada\n";
        cout << "2 - Registrar saida para obra\n";
        cout << "3 - Ajuste de estoque\n";
        cout << "4 - Cadastrar material\n";
        cout << "0 - Voltar ao menu principal\n";
        cout << "Escolha uma opcao: ";

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida. Digite um numero do menu.\n";
            continue;
        }

        switch (opcao) {
            case 1: registrarEntrada(); break; // RF02
            case 2: registrarSaidaObra(); break; // RF03
            case 3: ajusteEstoque(); break; // RF06/RF09
            case 4: cadastrarMaterial(); break; // RF01
            case 0: break;
            default: cout << "Opcao invalida. Digite um numero do menu.\n"; break;
        }
    }
}

// 1.1 Registrar Entrada
void registrarEntrada() {
    vector<Material> materiais = carregarMateriais();
    cout << "\n--- AIR SOLUTION ALMOXARIFADO REGISTRAR ENTRADA ---\n";

    string codigo;
    int idx = -1;

    while (true) {
        cout << "Codigo do material (0 = voltar): ";
        cin >> codigo;
        if (codigo == "0") return;

        idx = buscarMaterialPorCodigo(materiais, codigo);
        if (idx == -1) {
            cout << "Material nao encontrado.\n"; // Tela 1.1-A
        } else {
            break;
        }
    }

    cout << "Descricao: " << materiais[idx].descricao << "\n";
    cout << "Unidade: " << materiais[idx].unidade << "\n";
    cout << "Saldo atual: " << materiais[idx].saldo << "\n";

    double qtd = 0;
    while (true) {
        cout << "Quantidade recebida: ";
        if (!(cin >> qtd) || qtd <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Quantidade invalida.\n";
        } else {
            break;
        }
    }

    string data, hora;
    while (true) {
        cout << "Data (DD/MM/AAAA): ";
        cin >> data;
        if (!validarData(data)) {
            cout << "Data invalida. Use DD/MM/AAAA\n";
        } else {
            break;
        }
    }

    while (true) {
        cout << "Hora (HH:MM): ";
        cin >> hora;
        if (!validarHora(hora)) {
            cout << "Hora invalida. Use HH:MM.\n";
        } else {
            break;
        }
    }

    // Tela 1.1-B Resumo e confirmação
    cout << "\nRESUMO DA ENTRADA\n";
    cout << "Material: " << materiais[idx].codigo << " - " << materiais[idx].descricao << "\n";
    cout << "Entrada: " << qtd << " " << materiais[idx].unidade << "\n";
    cout << "Saldo: " << materiais[idx].saldo << " -> " << (materiais[idx].saldo + qtd) << "\n";
    cout << "Data: " << data << " " << hora << "\n";

    char conf;
    cout << "Confirmar entrada? (S/N): ";
    cin >> conf;

    if (toupper(conf) == 'S') {
        double saldoAnterior = materiais[idx].saldo;
        materiais[idx].saldo += qtd;
        salvarMateriais(materiais);

        // Registro em movimentacoes.csv
        stringstream ss;
        ss << data << " " << hora << ";ENTRADA;" << materiais[idx].codigo << ";" 
           << qtd << ";" << saldoAnterior << ";" << materiais[idx].saldo << ";Entrada de estoque";
        registrarMovimentacao(ss.str());

        cout << "\nEntrada registrada. Novo saldo: " << materiais[idx].saldo << " " << materiais[idx].unidade << "\n";
    } else {
        cout << "Entrada cancelada.\n";
    }
    pausar();
}

// 1.2 Registrar Saída para Obra
void registrarSaidaObra() {
    vector<Material> materiais = carregarMateriais();
    cout << "\n--- AIR SOLUTION ALMOXARIFADO SAIDA PARA OBRA ---\n";

    string obra = lerEntradaSemSemicolon("Codigo da obra ou manutencao (ex.: OP2035; 0 = voltar): ");
    if (obra == "0") return;

    string data, hora;
    while (true) {
        cout << "Data (DD/MM/AAAA): ";
        cin >> data;
        if (!validarData(data)) cout << "Data invalida. Use DD/MM/AAAA\n";
        else break;
    }

    while (true) {
        cout << "Hora (HH:MM): ";
        cin >> hora;
        if (!validarHora(hora)) cout << "Hora invalida. Use HH:MM.\n";
        else break;
    }

    vector<ItemSaida> itensSaida;
    char continuar = 'S';

    // Loop para inclusão de múltiplos materiais (Fluxograma 1.2)
    while (toupper(continuar) == 'S') {
        string codMat;
        cout << "\nCodigo do material (0 = cancelar a saida): ";
        cin >> codMat;

        if (codMat == "0") {
            cout << "Saida cancelada. Nenhum item foi gravado.\n";
            pausar();
            return;
        }

        int idx = buscarMaterialPorCodigo(materiais, codMat);
        if (idx == -1) {
            cout << "Material nao encontrado.\n";
            continue;
        }

        // Soma itens já incluídos na sessão para validação de estoque acumulado
        double jaIncluido = 0;
        for (const auto& item : itensSaida) {
            if (item.codigo == codMat) jaIncluido += item.quantidade;
        }

        double disponivel = materiais[idx].saldo - jaIncluido;
        if (disponivel <= 0) {
            cout << codMat << " " << materiais[idx].descricao << " Unidade: " << materiais[idx].unidade << " | Saldo: " << materiais[idx].saldo << "\n";
            cout << "Material sem saldo disponivel. Item nao incluido.\n";
            continue;
        }

        cout << codMat << " " << materiais[idx].descricao << " Unidade: " << materiais[idx].unidade << " | Saldo: " << materiais[idx].saldo;
        if (jaIncluido > 0) cout << " | Ja incluido: " << jaIncluido;
        cout << "\n";

        double qtd = 0;
        while (true) {
            cout << "Quantidade: ";
            if (!(cin >> qtd) || qtd <= 0) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Quantidade deve ser maior que zero.\n";
            } else if (qtd > disponivel) {
                cout << "Saldo insuficiente. Disponivel: " << disponivel << "\n"; // Trata saldo insuficiente
            } else {
                break;
            }
        }

        ItemSaida item;
        item.codigo = codMat;
        item.quantidade = qtd;
        itensSaida.push_back(item);

        cout << "Item incluido.\n";
        cout << "Incluir mais materiais? (S/N): ";
        cin >> continuar;
    }

    if (itensSaida.empty()) return;

    // Resumo final de Saída
    cout << "\n==========================================\n";
    cout << "RESUMO DA SAIDA OBRA " << obra << "\n";
    cout << "Data: " << data << " " << hora << "\n";
    cout << "COD\tDESCRICAO\tQTD\tUN\n";
    for (const auto& item : itensSaida) {
        int idx = buscarMaterialPorCodigo(materiais, item.codigo);
        cout << item.codigo << "\t" << materiais[idx].descricao << "\t" << item.quantidade << "\t" << materiais[idx].unidade << "\n";
    }

    char conf;
    cout << "\nConfirmar saida? (S/N): ";
    cin >> conf;

    if (toupper(conf) == 'S') {
        // Atualiza estoque e registra no arquivo CSV por item
        for (const auto& item : itensSaida) {
            int idx = buscarMaterialPorCodigo(materiais, item.codigo);
            double saldoAnt = materiais[idx].saldo;
            materiais[idx].saldo -= item.quantidade;

            stringstream ss;
            ss << data << " " << hora << ";SAIDA;" << item.codigo << ";" 
               << item.quantidade << ";" << saldoAnt << ";" << materiais[idx].saldo << ";" << obra;
            registrarMovimentacao(ss.str());
        }

        salvarMateriais(materiais);
        cout << "Saida registrada para a obra " << obra << " (" << itensSaida.size() << " itens).\n";
    } else {
        cout << "Saida cancelada.\n";
    }
    pausar();
}

// 1.3 Ajuste de Estoque
void ajusteEstoque() {
    vector<Material> materiais = carregarMateriais();
    cout << "\n--- AIR SOLUTION ALMOXARIFADO AJUSTE DE ESTOQUE ---\n";

    string codigo;
    int idx = -1;

    while (true) {
        cout << "Codigo do material (0 = voltar): ";
        cin >> codigo;
        if (codigo == "0") return;

        idx = buscarMaterialPorCodigo(materiais, codigo);
        if (idx == -1) {
            cout << "Material nao encontrado.\n";
        } else {
            break;
        }
    }

    cout << "Descricao: " << materiais[idx].descricao << " | Unidade: " << materiais[idx].unidade << "\n";
    cout << "Saldo no sistema: " << materiais[idx].saldo << "\n";

    double novoTotal = 0;
    while (true) {
        cout << "Nova quantidade total (valor contado): ";
        if (!(cin >> novoTotal) || novoTotal < 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Quantidade invalida.\n";
        } else if (novoTotal == materiais[idx].saldo) {
            cout << "Saldo ja confere. Nenhum ajuste necessario.\n";
            pausar();
            return;
        } else {
            break;
        }
    }

    int motivoOp = 0;
    string motivoStr, justificativa = "";
    string obraOrigem = "";

    while (true) {
        cout << "Motivo do ajuste:\n1 - Contagem fisica\n2 - Sobra de obra\nEscolha: ";
        if (!(cin >> motivoOp) || (motivoOp != 1 && motivoOp != 2)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida.\n";
        } else {
            break;
        }
    }

    if (motivoOp == 1) {
        motivoStr = "Contagem fisica";
        cin.ignore(10000, '\n');
        cout << "Justificativa (opcional): ";
        getline(cin, justificativa);
        if (contemPontoEVirgula(justificativa)) {
            replace(justificativa.begin(), justificativa.end(), ';', ',');
        }
    } else {
        motivoStr = "Sobra de obra";
        if (novoTotal <= materiais[idx].saldo) {
            cout << "Para sobra de obra, o novo total deve ser maior que o saldo atual.\n";
            pausar();
            return;
        }
        obraOrigem = lerEntradaSemSemicolon("Codigo da obra de origem (ex.: OP2035): ");
        cin.ignore(10000, '\n');
        cout << "Justificativa (opcional): ";
        getline(cin, justificativa);
        if (justificativa.empty()) {
            justificativa = "Retorno da " + obraOrigem;
        }
    }

    string data, hora;
    while (true) {
        cout << "Data (DD/MM/AAAA): ";
        cin >> data;
        if (!validarData(data)) cout << "Data invalida. Use DD/MM/AAAA\n";
        else break;
    }

    while (true) {
        cout << "Hora (HH:MM): ";
        cin >> hora;
        if (!validarHora(hora)) cout << "Hora invalida. Use HH:MM.\n";
        else break;
    }

    double diferenca = novoTotal - materiais[idx].saldo;

    // Resumo de ajuste
    cout << "\nRESUMO DO AJUSTE\n";
    cout << "Motivo: " << motivoStr << "\n";
    cout << "Saldo anterior: " << materiais[idx].saldo << "\n";
    cout << "Novo saldo: " << novoTotal << "\n";
    cout << "Diferenca: " << (diferenca > 0 ? "+" : "") << diferenca << "\n";
    cout << "Data: " << data << " " << hora << "\n";

    char conf;
    cout << "Confirmar ajuste? (S/N): ";
    cin >> conf;

    if (toupper(conf) == 'S') {
        double saldoAnt = materiais[idx].saldo;
        materiais[idx].saldo = novoTotal;
        salvarMateriais(materiais);

        stringstream ss;
        ss << data << " " << hora << ";AJUSTE;" << materiais[idx].codigo << ";" 
           << diferenca << ";" << saldoAnt << ";" << materiais[idx].saldo << ";" << motivoStr << " - " << justificativa;
        registrarMovimentacao(ss.str());

        cout << "\nAjuste registrado. Novo saldo: " << materiais[idx].saldo << " " << materiais[idx].unidade << "\n";
    } else {
        cout << "Ajuste cancelado.\n";
    }
    pausar();
}

// 1.4 Cadastrar Material
void cadastrarMaterial() {
    vector<Material> materiais = carregarMateriais();
    cout << "\n--- AIR SOLUTION ALMOXARIFADO CADASTRAR MATERIAL ---\n";

    string codigo = lerEntradaSemSemicolon("Codigo do material (0 = voltar): ");
    if (codigo == "0") return;

    if (buscarMaterialPorCodigo(materiais, codigo) != -1) {
        cout << "Material ja cadastrado.\n";
        pausar();
        return;
    }

    cin.ignore(10000, '\n');
    string desc;
    while (true) {
        cout << "Descricao (maquina: modelo e S/N): ";
        getline(cin, desc);
        if (desc.empty()) cout << "A descricao e obrigatoria.\n";
        else if (contemPontoEVirgula(desc)) cout << "A descricao nao pode conter ponto e virgula.\n";
        else break;
    }

    string uni;
    while (true) {
        cout << "Unidade (UN ou M): ";
        cin >> uni;
        transform(uni.begin(), uni.end(), uni.begin(), ::toupper);
        if (uni != "UN" && uni != "M") {
            cout << "Unidade invalida. Digite UN ou M.\n";
        } else {
            break;
        }
    }

    double qtdInicial = 0;
    while (true) {
        cout << "Quantidade inicial em estoque (pode ser 0): ";
        if (!(cin >> qtdInicial) || qtdInicial < 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Saldo negativo.\n";
        } else {
            break;
        }
    }

    string data = "", hora = "";
    if (qtdInicial > 0) {
        while (true) {
            cout << "Data (DD/MM/AAAA): ";
            cin >> data;
            if (!validarData(data)) cout << "Data invalida. Use DD/MM/AAAA\n";
            else break;
        }

        while (true) {
            cout << "Hora (HH:MM): ";
            cin >> hora;
            if (!validarHora(hora)) cout << "Hora invalida. Use HH:MM.\n";
            else break;
        }
    }

    // Resumo
    cout << "\nRESUMO DO CADASTRO\n";
    cout << "Codigo: " << codigo << "\n";
    cout << "Descricao: " << desc << "\n";
    cout << "Unidade: " << uni << "\n";
    cout << "Saldo inicial: " << qtdInicial << "\n";

    char conf;
    cout << "Confirmar cadastro? (S/N): ";
    cin >> conf;

    if (toupper(conf) == 'S') {
        Material novoMat = {codigo, desc, uni, qtdInicial};
        materiais.push_back(novoMat);
        salvarMateriais(materiais);

        if (qtdInicial > 0) {
            stringstream ss;
            ss << data << " " << hora << ";ENTRADA;" << codigo << ";" 
               << qtdInicial << ";0;" << qtdInicial << ";Saldo inicial";
            registrarMovimentacao(ss.str());
        }

        cout << "\nMaterial cadastrado com sucesso.\n";
    } else {
        cout << "Cadastro cancelado.\n";
    }
    pausar();
}

// ==========================================
// MÓDULO 2: ACOMPANHAMENTO DE OBRAS
// ==========================================

void menuAcompanhamentoObras() {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n------------------------------------------\n";
        cout << " AIR SOLUTION ACOMPANHAMENTO DE OBRAS     \n";
        cout << "------------------------------------------\n";
        cout << "1 - Materiais enviados por obra\n";
        cout << "0 - Voltar ao menu principal\n";
        cout << "Escolha uma opcao: ";

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida. Digite um numero do menu.\n";
            continue;
        }

        switch (opcao) {
            case 1: materiaisEnviadosPorObra(); break; // RF03 consulta
            case 0: break;
            default: cout << "Opcao invalida. Digite um numero do menu.\n"; break;
        }
    }
}

// 2.1 Materiais enviados por Obra
void materiaisEnviadosPorObra() {
    cout << "\n--- AIR SOLUTION OBRAS MATERIAIS ENVIADOS POR OBRA ---\n";
    string obra = lerEntradaSemSemicolon("Codigo da obra ou manutencao (0 = voltar): ");
    if (obra == "0") return;

    vector<Material> materiais = carregarMateriais();
    ifstream arqMov("movimentacoes.csv");

    if (!arqMov.is_open()) {
        cout << "Nenhuma movimentacao encontrada.\n";
        pausar();
        return;
    }

    string linha;
    int encontradas = 0;
    cout << "\n" << left << setw(12) << "DATA" << setw(8) << "HORA" << setw(8) << "COD" 
         << setw(30) << "DESCRICAO" << setw(8) << "QTD" << "UN\n";
    cout << "-------------------------------------------------------------------\n";

    while (getline(arqMov, linha)) {
        if (linha.empty()) continue;
        stringstream ss(linha);
        string dataHora, tipo, cod, qtdStr, salAnt, salNovo, infoObra;

        getline(ss, dataHora, ';');
        getline(ss, tipo, ';');
        getline(ss, cod, ';');
        getline(ss, qtdStr, ';');
        getline(ss, salAnt, ';');
        getline(ss, salNovo, ';');
        getline(ss, infoObra, ';');

        // Filtra por tipo SAIDA e pelo código da obra
        if (tipo == "SAIDA" && infoObra == obra) {
            encontradas++;
            string data = dataHora.substr(0, 10);
            string hora = (dataHora.length() >= 16) ? dataHora.substr(11, 5) : "";

            int idx = buscarMaterialPorCodigo(materiais, cod);
            string desc = (idx != -1) ? materiais[idx].descricao : "N/A";
            string uni = (idx != -1) ? materiais[idx].unidade : "N/A";

            cout << left << setw(12) << data << setw(8) << hora << setw(8) << cod 
                 << setw(30) << desc << setw(8) << qtdStr << uni << "\n";
        }
    }
    arqMov.close();

    if (encontradas == 0) {
        cout << "Nenhuma saida encontrada para a " << obra << ".\n";
    } else {
        cout << "\n" << encontradas << " saida(s) encontrada(s).\n";
    }
    pausar();
}

// ==========================================
// MÓDULO 3: GESTÃO DO ESTOQUE
// ==========================================

void menuGestaoEstoque() {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n------------------------------------------\n";
        cout << "       AIR SOLUTION GESTAO DO ESTOQUE     \n";
        cout << "------------------------------------------\n";
        cout << "1 - Saldo de todos os materiais\n";
        cout << "2 - Pesquisar material por codigo ou descricao\n";
        cout << "0 - Voltar ao menu principal\n";
        cout << "Escolha uma opcao: ";

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida. Digite um numero do menu.\n";
            continue;
        }

        switch (opcao) {
            case 1: saldoTodosMateriais(); break; // RF04
            case 2: pesquisarMaterial(); break; // RF04
            case 0: break;
            default: cout << "Opcao invalida. Digite um numero do menu.\n"; break;
        }
    }
}

// 3.1 Saldo de Todos os Materiais
void saldoTodosMateriais() {
    vector<Material> materiais = carregarMateriais();
    cout << "\n==========================================\n";
    cout << "AIR SOLUTION GESTAO DO ESTOQUE > SALDO DE MATERIAIS\n";
    cout << "==========================================\n";

    if (materiais.empty()) {
        cout << "Nenhum material cadastrado.\n";
    } else {
        cout << left << setw(10) << "CODIGO" << setw(32) << "DESCRICAO" << setw(6) << "UN" << "SALDO\n";
        cout << "---------------------------------------------------------\n";
        for (const auto& m : materiais) {
            cout << left << setw(10) << m.codigo << setw(32) << m.descricao << setw(6) << m.unidade << m.saldo << "\n";
        }
        cout << "\n" << materiais.size() << " material(is) cadastrado(s).\n";
    }
    pausar();
}

// 3.2 Pesquisar Material
void pesquisarMaterial() {
    int tipoBusca = -1;
    while (tipoBusca != 0) {
        cout << "\n------------------------------------------\n";
        cout << "AIR SOLUTION GESTAO DO ESTOQUE > PESQUISAR MATERIAL\n";
        cout << "------------------------------------------\n";
        cout << "Pesquisar por:\n1 - Codigo\n2 - Descricao\n0 - Voltar\nEscolha: ";

        if (!(cin >> tipoBusca)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcao invalida.\n";
            continue;
        }

        if (tipoBusca == 0) break;
        if (tipoBusca != 1 && tipoBusca != 2) {
            cout << "Opcao invalida.\n";
            continue;
        }

        cin.ignore(10000, '\n');
        string termo;
        cout << "Termo de busca (codigo ou parte da descricao): ";
        getline(cin, termo);

        vector<Material> materiais = carregarMateriais();
        int encontradas = 0;

        cout << "\n" << left << setw(10) << "CODIGO" << setw(32) << "DESCRICAO" << setw(6) << "UN" << "SALDO\n";
        cout << "---------------------------------------------------------\n";

        string termoMin = paraMinusculo(termo);

        for (const auto& m : materiais) {
            bool bateu = false;
            if (tipoBusca == 1 && m.codigo == termo) {
                bateu = true; // Exata por Código
            } else if (tipoBusca == 2 && paraMinusculo(m.descricao).find(termoMin) != string::npos) {
                bateu = true; // Parcial insensível a maiúsculas por Descrição
            }

            if (bateu) {
                encontradas++;
                cout << left << setw(10) << m.codigo << setw(32) << m.descricao << setw(6) << m.unidade << m.saldo << "\n";
            }
        }

        if (encontradas == 0) {
            cout << "Nenhum material encontrado.\n";
        } else {
            cout << "\n" << encontradas << " material(is) encontrado(s).\n";
        }
    }
}
