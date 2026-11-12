#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace std;

struct Material { string codigo, descricao, unidade; double saldo; }; // unidade: UN ou M
typedef void (*Acao)();

const string ERRO_PV = "O campo nao pode conter ponto e virgula.";

// ==========================================
// UTILITARIOS
// ==========================================

// Limpa a tela (e o historico de rolagem) do terminal
void limparTela() {
#ifdef _WIN32
    // Ativa o suporte a sequencias ANSI no console do Windows
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo))
        SetConsoleMode(h, modo | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
#endif
    // 2J = limpa a tela | 3J = limpa o historico de rolagem | H = cursor no topo
    cout << "\033[2J\033[3J\033[H" << flush;
}

// descartarResto = true  -> depois de "cin >> x" (sobra um ENTER no buffer)
// descartarResto = false -> depois de "getline" (buffer ja esta limpo)
void pausar(bool descartarResto = true) {
    cout << "\nPressione ENTER para continuar...";
    if (descartarResto) cin.ignore(10000, '\n');
    cin.get();
}

void tela(const string& titulo) {
    limparTela();
    cout << "\n--- AIR SOLUTION " << titulo << " ---\n";
}

string caixa(string s, int (*f)(int)) {
    transform(s.begin(), s.end(), s.begin(), [f](unsigned char c) { return static_cast<char>(f(c)); });
    return s;
}

// Confere o texto contra um molde: 'D' = digito, qualquer outro caractere deve ser igual
bool formato(const string& s, const string& molde) {
    if (s.size() != molde.size()) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (molde[i] == 'D' ? !isdigit(static_cast<unsigned char>(s[i])) : s[i] != molde[i]) return false;
    return true;
}

bool semPontoEVirgula(const string& s) { return s.find(';') == string::npos; }
bool dataOk(const string& d) { return formato(d, "DD/DD/DDDD"); }
bool horaOk(const string& h) { return formato(h, "DD:DD") && stoi(h.substr(0, 2)) <= 23 && stoi(h.substr(3, 2)) <= 59; }
bool unidadeOk(const string& s) { string u = caixa(s, ::toupper); return u == "UN" || u == "M"; }

// Le uma palavra ate ser valida
string ler(const string& msg, bool (*ok)(const string&), const string& erro) {
    string s;
    while (true) {
        cout << msg;
        cin >> s;
        if (ok(s)) return s;
        cout << erro << "\n";
    }
}

// Le um numero (> 0, ou >= 0 se aceitaZero) ate ser valido
double lerNumero(const string& msg, const string& erro, bool aceitaZero) {
    double v;
    while (true) {
        cout << msg;
        if (cin >> v && (aceitaZero ? v >= 0 : v > 0)) return v;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << erro << "\n";
    }
}

string lerDataHora() {
    string d = ler("Data (DD/MM/AAAA): ", dataOk, "Data invalida. Use DD/MM/AAAA");
    string h = ler("Hora (HH:MM): ", horaOk, "Hora invalida. Use HH:MM.");
    return d + " " + h;
}

bool confirmar(const string& msg) {
    char c;
    cout << msg << " (S/N): ";
    cin >> c;
    return toupper(c) == 'S';
}

// ==========================================
// ARQUIVOS CSV
// ==========================================

// materiais.csv: codigo;descricao;unidade;saldo
vector<Material> carregarMateriais() {
    vector<Material> v;
    ifstream f("materiais.csv");
    string linha;
    while (getline(f, linha)) {
        stringstream ss(linha);
        Material m;
        string saldo;
        if (getline(ss, m.codigo, ';') && getline(ss, m.descricao, ';') &&
            getline(ss, m.unidade, ';') && getline(ss, saldo, ';')) {
            m.saldo = stod(saldo);
            v.push_back(m);
        }
    }
    return v;
}

void salvarMateriais(const vector<Material>& v) {
    ofstream f("materiais.csv", ios::trunc);
    for (const auto& m : v)
        f << m.codigo << ";" << m.descricao << ";" << m.unidade << ";" << fixed << setprecision(2) << m.saldo << "\n";
}

// movimentacoes.csv: data hora;tipo;codigo;quantidade;saldo anterior;saldo novo;observacao
void registrarMovimentacao(const string& dataHora, const string& tipo, const string& cod,
                           double qtd, double anterior, double novo, const string& obs) {
    ofstream f("movimentacoes.csv", ios::app);
    f << dataHora << ";" << tipo << ";" << cod << ";" << qtd << ";" << anterior << ";" << novo << ";" << obs << "\n";
}

int buscarMaterialPorCodigo(const vector<Material>& v, const string& cod) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i].codigo == cod) return static_cast<int>(i);
    return -1;
}

// Pede o codigo ate achar um material. Retorna o indice, ou -1 se o usuario digitar 0
int escolherMaterial(const vector<Material>& v) {
    while (true) {
        string cod;
        cout << "Codigo do material (0 = voltar): ";
        cin >> cod;
        if (cod == "0") return -1;
        int i = buscarMaterialPorCodigo(v, cod);
        if (i >= 0) return i;
        cout << "Material nao encontrado.\n";
    }
}

void tabelaMateriais(const vector<Material>& v) {
    cout << left << setw(10) << "CODIGO" << setw(32) << "DESCRICAO" << setw(6) << "UN" << "SALDO\n"
         << string(57, '-') << "\n";
    for (const auto& m : v)
        cout << left << setw(10) << m.codigo << setw(32) << m.descricao << setw(6) << m.unidade << m.saldo << "\n";
}

// ==========================================
// MENU GENERICO (um unico codigo para todos os menus)
// ==========================================

void menu(const string& titulo, char borda, const vector<pair<string, Acao>>& itens, const string& saida) {
    int op = -1;
    string aviso;
    while (op != 0) {
        limparTela();
        cout << "\n" << string(42, borda) << "\n" << titulo << "\n" << string(42, borda) << "\n";
        for (size_t i = 0; i < itens.size(); ++i) cout << i + 1 << " - " << itens[i].first << "\n";
        cout << "0 - " << saida << "\n";
        if (!aviso.empty()) cout << "\n" << aviso << "\n";
        aviso.clear();
        cout << "Escolha uma opcao: ";

        if (!(cin >> op)) { // o cin zera a variavel ao falhar; -1 evita sair do menu
            cin.clear();
            cin.ignore(10000, '\n');
            op = -1;
        }
        if (op > 0 && op <= static_cast<int>(itens.size())) itens[op - 1].second();
        else if (op != 0) aviso = "Opcao invalida. Digite um numero do menu.";
    }
}

// ==========================================
// MODULO 1: ALMOXARIFADO
// ==========================================

void registrarEntrada() {
    vector<Material> v = carregarMateriais();
    tela("ALMOXARIFADO REGISTRAR ENTRADA");
    int i = escolherMaterial(v);
    if (i < 0) return;
    Material& m = v[i];

    cout << "Descricao: " << m.descricao << "\nUnidade: " << m.unidade << "\nSaldo atual: " << m.saldo << "\n";
    double qtd = lerNumero("Quantidade recebida: ", "Quantidade invalida.", false);
    string dh = lerDataHora();

    cout << "\nRESUMO DA ENTRADA\nMaterial: " << m.codigo << " - " << m.descricao
         << "\nEntrada: " << qtd << " " << m.unidade
         << "\nSaldo: " << m.saldo << " -> " << m.saldo + qtd << "\nData: " << dh << "\n";

    if (confirmar("Confirmar entrada?")) {
        double anterior = m.saldo;
        m.saldo += qtd;
        salvarMateriais(v);
        registrarMovimentacao(dh, "ENTRADA", m.codigo, qtd, anterior, m.saldo, "Entrada de estoque");
        cout << "\nEntrada registrada. Novo saldo: " << m.saldo << " " << m.unidade << "\n";
    } else {
        cout << "Entrada cancelada.\n";
    }
    pausar();
}

void registrarSaidaObra() {
    vector<Material> v = carregarMateriais();
    tela("ALMOXARIFADO SAIDA PARA OBRA");
    string obra = ler("Codigo da obra ou manutencao (ex.: OP2035; 0 = voltar): ", semPontoEVirgula, ERRO_PV);
    if (obra == "0") return;
    string dh = lerDataHora();

    vector<pair<int, double>> itens; // (indice do material, quantidade)
    char mais = 'S';
    while (toupper(mais) == 'S') {
        string cod;
        cout << "\nCodigo do material (0 = cancelar a saida): ";
        cin >> cod;
        if (cod == "0") {
            cout << "Saida cancelada. Nenhum item foi gravado.\n";
            pausar();
            return;
        }
        int i = buscarMaterialPorCodigo(v, cod);
        if (i < 0) { cout << "Material nao encontrado.\n"; continue; }

        // Desconta o que ja foi incluido nesta saida
        double ja = 0;
        for (const auto& it : itens) if (it.first == i) ja += it.second;
        double disponivel = v[i].saldo - ja;

        cout << cod << " " << v[i].descricao << " Unidade: " << v[i].unidade << " | Saldo: " << v[i].saldo;
        if (ja > 0) cout << " | Ja incluido: " << ja;
        cout << "\n";
        if (disponivel <= 0) { cout << "Material sem saldo disponivel. Item nao incluido.\n"; continue; }

        double qtd;
        while ((qtd = lerNumero("Quantidade: ", "Quantidade deve ser maior que zero.", false)) > disponivel)
            cout << "Saldo insuficiente. Disponivel: " << disponivel << "\n";
        itens.push_back({i, qtd});

        cout << "Item incluido.\nIncluir mais materiais? (S/N): ";
        cin >> mais;
    }

    cout << "\n" << string(42, '=') << "\nRESUMO DA SAIDA OBRA " << obra << "\nData: " << dh << "\nCOD\tDESCRICAO\tQTD\tUN\n";
    for (const auto& it : itens)
        cout << v[it.first].codigo << "\t" << v[it.first].descricao << "\t" << it.second << "\t" << v[it.first].unidade << "\n";

    if (confirmar("\nConfirmar saida?")) {
        for (const auto& it : itens) {
            Material& m = v[it.first];
            double anterior = m.saldo;
            m.saldo -= it.second;
            registrarMovimentacao(dh, "SAIDA", m.codigo, it.second, anterior, m.saldo, obra);
        }
        salvarMateriais(v);
        cout << "Saida registrada para a obra " << obra << " (" << itens.size() << " itens).\n";
    } else {
        cout << "Saida cancelada.\n";
    }
    pausar();
}

void ajusteEstoque() {
    vector<Material> v = carregarMateriais();
    tela("ALMOXARIFADO AJUSTE DE ESTOQUE");
    int i = escolherMaterial(v);
    if (i < 0) return;
    Material& m = v[i];

    cout << "Descricao: " << m.descricao << " | Unidade: " << m.unidade << "\nSaldo no sistema: " << m.saldo << "\n";
    double novo = lerNumero("Nova quantidade total (valor contado): ", "Quantidade invalida.", true);
    if (novo == m.saldo) {
        cout << "Saldo ja confere. Nenhum ajuste necessario.\n";
        pausar();
        return;
    }

    int motivo;
    while (true) {
        cout << "Motivo do ajuste:\n1 - Contagem fisica\n2 - Sobra de obra\nEscolha: ";
        if (cin >> motivo && (motivo == 1 || motivo == 2)) break;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Opcao invalida.\n";
    }

    string motivoStr = (motivo == 1) ? "Contagem fisica" : "Sobra de obra", obra, just;
    if (motivo == 2) {
        if (novo <= m.saldo) {
            cout << "Para sobra de obra, o novo total deve ser maior que o saldo atual.\n";
            pausar();
            return;
        }
        obra = ler("Codigo da obra de origem (ex.: OP2035): ", semPontoEVirgula, ERRO_PV);
    }
    cin.ignore(10000, '\n');
    cout << "Justificativa (opcional): ";
    getline(cin, just);
    replace(just.begin(), just.end(), ';', ','); // ';' e o separador do CSV
    if (motivo == 2 && just.empty()) just = "Retorno da " + obra;

    string dh = lerDataHora();
    double dif = novo - m.saldo;

    cout << "\nRESUMO DO AJUSTE\nMotivo: " << motivoStr << "\nSaldo anterior: " << m.saldo << "\nNovo saldo: " << novo
         << "\nDiferenca: " << (dif > 0 ? "+" : "") << dif << "\nData: " << dh << "\n";

    if (confirmar("Confirmar ajuste?")) {
        double anterior = m.saldo;
        m.saldo = novo;
        salvarMateriais(v);
        registrarMovimentacao(dh, "AJUSTE", m.codigo, dif, anterior, novo, motivoStr + " - " + just);
        cout << "\nAjuste registrado. Novo saldo: " << m.saldo << " " << m.unidade << "\n";
    } else {
        cout << "Ajuste cancelado.\n";
    }
    pausar();
}

void cadastrarMaterial() {
    vector<Material> v = carregarMateriais();
    tela("ALMOXARIFADO CADASTRAR MATERIAL");
    string cod = ler("Codigo do material (0 = voltar): ", semPontoEVirgula, ERRO_PV);
    if (cod == "0") return;
    if (buscarMaterialPorCodigo(v, cod) >= 0) {
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
        else if (!semPontoEVirgula(desc)) cout << "A descricao nao pode conter ponto e virgula.\n";
        else break;
    }

    string uni = caixa(ler("Unidade (UN ou M): ", unidadeOk, "Unidade invalida. Digite UN ou M."), ::toupper);
    double qtd = lerNumero("Quantidade inicial em estoque (pode ser 0): ", "Saldo negativo.", true);
    string dh = (qtd > 0) ? lerDataHora() : "";

    cout << "\nRESUMO DO CADASTRO\nCodigo: " << cod << "\nDescricao: " << desc
         << "\nUnidade: " << uni << "\nSaldo inicial: " << qtd << "\n";

    if (confirmar("Confirmar cadastro?")) {
        v.push_back({cod, desc, uni, qtd});
        salvarMateriais(v);
        if (qtd > 0) registrarMovimentacao(dh, "ENTRADA", cod, qtd, 0, qtd, "Saldo inicial");
        cout << "\nMaterial cadastrado com sucesso.\n";
    } else {
        cout << "Cadastro cancelado.\n";
    }
    pausar();
}

void menuAlmoxarifado() {
    menu("AIR SOLUTION ALMOXARIFADO", '-',
         {{"Registrar entrada", registrarEntrada}, {"Registrar saida para obra", registrarSaidaObra},
          {"Ajuste de estoque", ajusteEstoque}, {"Cadastrar material", cadastrarMaterial}},
         "Voltar ao menu principal");
}

// ==========================================
// MODULO 2: ACOMPANHAMENTO DE OBRAS
// ==========================================

void materiaisEnviadosPorObra() {
    tela("OBRAS MATERIAIS ENVIADOS POR OBRA");
    string obra = ler("Codigo da obra ou manutencao (0 = voltar): ", semPontoEVirgula, ERRO_PV);
    if (obra == "0") return;

    ifstream f("movimentacoes.csv");
    if (!f) {
        cout << "Nenhuma movimentacao encontrada.\n";
        pausar();
        return;
    }

    vector<Material> v = carregarMateriais();
    cout << "\n" << left << setw(12) << "DATA" << setw(8) << "HORA" << setw(8) << "COD" << setw(30) << "DESCRICAO"
         << setw(8) << "QTD" << "UN\n" << string(67, '-') << "\n";

    string linha;
    int encontradas = 0;
    while (getline(f, linha)) {
        stringstream ss(linha);
        string dh, tipo, cod, qtd, anterior, novo, info;
        for (string* campo : {&dh, &tipo, &cod, &qtd, &anterior, &novo, &info}) getline(ss, *campo, ';');
        if (tipo != "SAIDA" || info != obra) continue;

        int i = buscarMaterialPorCodigo(v, cod);
        cout << left << setw(12) << dh.substr(0, 10) << setw(8) << (dh.size() >= 16 ? dh.substr(11, 5) : "")
             << setw(8) << cod << setw(30) << (i >= 0 ? v[i].descricao : "N/A") << setw(8) << qtd
             << (i >= 0 ? v[i].unidade : "N/A") << "\n";
        ++encontradas;
    }

    if (encontradas == 0) cout << "Nenhuma saida encontrada para a " << obra << ".\n";
    else cout << "\n" << encontradas << " saida(s) encontrada(s).\n";
    pausar();
}

void menuAcompanhamentoObras() {
    menu("AIR SOLUTION ACOMPANHAMENTO DE OBRAS", '-',
         {{"Materiais enviados por obra", materiaisEnviadosPorObra}}, "Voltar ao menu principal");
}

// ==========================================
// MODULO 3: GESTAO DO ESTOQUE
// ==========================================

void saldoTodosMateriais() {
    vector<Material> v = carregarMateriais();
    tela("GESTAO DO ESTOQUE > SALDO DE MATERIAIS");
    if (v.empty()) {
        cout << "Nenhum material cadastrado.\n";
    } else {
        tabelaMateriais(v);
        cout << "\n" << v.size() << " material(is) cadastrado(s).\n";
    }
    pausar();
}

// Por codigo: busca exata | Por descricao: busca parcial, sem diferenciar maiusculas
void pesquisar(bool porCodigo) {
    tela("GESTAO DO ESTOQUE > PESQUISAR MATERIAL");
    string termo;
    cout << "Termo de busca (codigo ou parte da descricao): ";
    cin.ignore(10000, '\n');
    getline(cin, termo);

    string t = caixa(termo, ::tolower);
    vector<Material> achados;
    for (const auto& m : carregarMateriais())
        if (porCodigo ? m.codigo == termo : caixa(m.descricao, ::tolower).find(t) != string::npos)
            achados.push_back(m);

    cout << "\n";
    tabelaMateriais(achados);
    if (achados.empty()) cout << "Nenhum material encontrado.\n";
    else cout << "\n" << achados.size() << " material(is) encontrado(s).\n";
    pausar(false);
}

void pesquisarPorCodigo() { pesquisar(true); }
void pesquisarPorDescricao() { pesquisar(false); }

void menuPesquisa() {
    menu("AIR SOLUTION GESTAO DO ESTOQUE > PESQUISAR MATERIAL", '-',
         {{"Pesquisar por codigo", pesquisarPorCodigo}, {"Pesquisar por descricao", pesquisarPorDescricao}}, "Voltar");
}

void menuGestaoEstoque() {
    menu("AIR SOLUTION GESTAO DO ESTOQUE", '-',
         {{"Saldo de todos os materiais", saldoTodosMateriais},
          {"Pesquisar material por codigo ou descricao", menuPesquisa}},
         "Voltar ao menu principal");
}

// ==========================================
// MAIN
// ==========================================

int main() {
    menu("AIR SOLUTION MENU PRINCIPAL", '=',
         {{"Almoxarifado", menuAlmoxarifado}, {"Acompanhamento de obras", menuAcompanhamentoObras},
          {"Gestao do estoque", menuGestaoEstoque}},
         "Sair");
    cout << "\nAir Solution - Menu principal\nSistema encerrado. Ate logo!\n";
    return 0;
}
