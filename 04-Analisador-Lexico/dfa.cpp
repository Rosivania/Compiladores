#include "dfa.h"

//-----------------------------------------------------------------------------
DFA::DFA(const NFATable& nfa, const std::vector<uchar>& Alfabeto, string token, string padrao):
    nfa(nfa), Alfabeto(Alfabeto), idAtual(0), token(token), padrao(padrao){}

//-----------------------------------------------------------------------------
void DFA::CriaEstadoDeErro()
{
    this->estadoErro = new DFAEstado(-1);

    for(int i = 0; i < (int)this->Alfabeto.size(); i++)
        this->estadoErro->transicoes.emplace(this->Alfabeto[i], this->estadoErro);

    estadoErro->estadoErro = true;
}

//-----------------------------------------------------------------------------
void DFA::renomeiaEstados()
{
    int i = 0;

    for(auto it = this->dfa.begin(); it != this->dfa.end(); ++it)
    {
        (*it)->id = i;
        i++;
    }
}

//-----------------------------------------------------------------------------
std::unordered_set<NFAEstado*> DFA::calculaFecho(NFAEstado* estadoAtual)
{
    // O fecho-& para o estado atual já foi calculado
    if(estadoAtual->fecho)
    {
        auto itFind = this->fechos.find(estadoAtual);
        return itFind->second;
    }

    // Define que o estado atual já foi calculado (antes msm de realmente calcular)
    estadoAtual->fecho = true;

    // Insere em fechos {fecho(qi) = {qi}}
    std::unordered_set<NFAEstado*> listaEstados;
    listaEstados.insert(estadoAtual);

    // Obtem o conjuto de estados imediatamente acessíveis ao estado atual por &-transição
    auto it = estadoAtual->transicoes.equal_range(EPSILON);

    // Chama a função calcula fecho de forma recursiva para cada estado imediatamente acessível
    // or &-transição
    for_each(
        it.first,
        it.second,
        [&](trans::value_type& x){
           auto aux = calculaFecho(x.second);// Chamada recursiva
           listaEstados.insert(aux.begin(), aux.end());
        }
    );

    auto itFind = this->fechos.find(estadoAtual);
    itFind->second.insert(listaEstados.begin(), listaEstados.end());
    return listaEstados;
}

//-----------------------------------------------------------------------------
// Percorre um fecho-&, procura e retorna o estado que tem transição para o símbolo passado
NFAEstado* DFA::moveFecho(NFAEstado* estadoAtual, uchar simbolo)
{
    auto itFind = this->fechos.find(estadoAtual);
    // Caso não haja transição do estadoAtual para o símbolo dado
    NFAEstado* novoEstado = NULL;

    //Percorre o fecho-& do estado atual
    for(auto it = itFind->second.begin(); it != itFind->second.end(); ++it)
    {
        auto itFind2 = (*it)->transicoes.equal_range(simbolo);

        for_each(
            itFind2.first,
            itFind2.second,
            [&novoEstado](trans::value_type& x){
                novoEstado = x.second;
        }
        );
    }

    return novoEstado;
}

//-----------------------------------------------------------------------------
void DFA::criarDFA()
{

    bool estadoExiste; // Utilizada pra verificar se um estado de transição já existe no DFA
    bool primeiroErro = true;

    // Remove o EPSILON do alfabeto
    this->Alfabeto.pop_back();

    //Inserir estados na lista de &-fechos
    for(auto it = this->nfa.begin(); it != this->nfa.end(); ++it)
    {
        this->fechos.insert(std::make_pair(*it, std::unordered_set<NFAEstado*>({})));
    }

    //Cria a lista de fechos
    for(auto it = this->nfa.begin(); it != this->nfa.end(); ++it)
    {
        calculaFecho(*(it));

        // Torna os estados não visitados para calcular o próximo fecho
        for(auto it2 = this->nfa.begin(); it2 != this->nfa.end(); ++it2)
        {
            (*it2)->fecho = false;
        }
    }

    // Cria o estado de erro (mesmo que não seja necessário)
    CriaEstadoDeErro();

    // Inicia a pilha com estado inicial
    auto it = fechos.begin();
    std::deque<NFAEstado*> listaDeEstadosNFA;
    listaDeEstadosNFA.push_front(it->first);

    // Cria e insere o estado inicial (sem as transições)
    DFAEstado* novoEstadoDFA = new DFAEstado(it->first->id);

    // Verifica se o primeiro estado do DFA é final
    for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        if((*it2)->estadoFinal)
            novoEstadoDFA->estadoFinal = true;

    // Insere o primeiro estado no DFA
    this->dfa.push_front(novoEstadoDFA);


    // Parte que cria o DFA
    while(!listaDeEstadosNFA.empty())
    {

        DFAEstado* atual;

        // Busca o elemento atual do DFA para inserir transições
        for(auto it = this->dfa.begin(); it != dfa.end(); ++it)
            if((*it)->id == listaDeEstadosNFA.front()->id)
                atual = *it;


        // Crias as transições
        for(int i = 0; i < (int)this->Alfabeto.size(); i++)
        {
            estadoExiste = false;

            NFAEstado* novoEstadoNFA = moveFecho(listaDeEstadosNFA.front(), this->Alfabeto[i]);

            // Existe transição para o símbolo atual
            if(novoEstadoNFA != NULL)
            {
                for(auto it = this->dfa.begin(); it != dfa.end(); ++it)
                    if((*it)->id == novoEstadoNFA->id)
                        estadoExiste = true;

                // O estado de transição é um estado novo
                if(!estadoExiste)
                {
                    novoEstadoDFA = new DFAEstado(novoEstadoNFA->id);
                    // Insere um novo estado
                    this->dfa.push_back(novoEstadoDFA);

                    // Insere a transição no estado atual
                    atual->transicoes.emplace(this->Alfabeto[i], novoEstadoDFA);

                    auto itFind = this->fechos.find(novoEstadoNFA);

                    // Verifica se o novo estado é inicial
                    for(auto it2 = itFind->second.begin(); it2 != itFind->second.end(); ++it2)
                        if((*it2)->estadoFinal)
                            novoEstadoDFA->estadoFinal = true;

                    listaDeEstadosNFA.push_back(novoEstadoNFA);

                }
                else // O estado de transição é um estado já existente
                {
                    for(auto it = dfa.begin(); it != dfa.end(); ++it)
                        if((*it)->id == novoEstadoNFA->id)
                             atual->transicoes.emplace(this->Alfabeto[i], *it);
                }
            }
            else // Não existe transição para o símbolo atual (vai para o estado)
            {
                atual->transicoes.emplace(this->Alfabeto[i], this->estadoErro);

                if(primeiroErro)
                {
                    this->dfa.push_back(this->estadoErro);
                    primeiroErro = false;
                }
            }

        }

        // Remove o estado atual (já criadas as transições no DFA)
        listaDeEstadosNFA.pop_front();
    }

    renomeiaEstados();
}

//-----------------------------------------------------------------------------
int DFA::funcaoDeTransicao(const string &codigo, int indiceAtual, DFAEstado *estadoAtual)
{   
    //Estado de Erro ou fim do codigo
    if(estadoAtual->estadoErro || indiceAtual >= codigo.size())
        return -1;

    //Busca a transição para o símbolo atual
    auto it = estadoAtual->transicoes.find(codigo[indiceAtual]);

    //cout << it->second->id << endl;

    if(it != estadoAtual->transicoes.end())
    {
        //chama a função de transição passando o estado de transição e próximo índice
        int a = funcaoDeTransicao(codigo, indiceAtual+1, it->second);


        //Retorno é um índice válido
        if(a != -1)
            return a;

        //O estado Atual é um estado final
        if(it->second->estadoFinal)
            return indiceAtual;
    }

    //Todos os estados até aqui não são de aceitação
    return -1;
}
//-----------------------------------------------------------------------------
