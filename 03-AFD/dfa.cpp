#include "dfa.h"

//--------------------------------------------------------------------------

DFA::DFA(const NFATable& nfa, const std::vector<uchar>& Alfabeto): nfa(nfa), Alfabeto(Alfabeto), idAtual(0){}

//--------------------------------------------------------------------------

std::unordered_set<NFAEstado*> DFA::calculaFecho(NFAEstado* estadoAtual)
{
    if(estadoAtual->fecho)
    {
        auto itFind = this->fechos.find(estadoAtual);
        return itFind->second;
    }

    estadoAtual->fecho = true;

    //insere em fechos {fecho(qi) = {qi}}
    std::unordered_set<NFAEstado*> listaEstados;
    listaEstados.insert(estadoAtual);

    auto it = estadoAtual->transicoes.equal_range(EPSILON);

    for_each(
        it.first,
        it.second,
        [&](trans::value_type& x){
           //itFind->second.insert(x.second);
           auto aux = calculaFecho(x.second);
           listaEstados.insert(aux.begin(), aux.end());
        }
        );

      auto itFind = this->fechos.find(estadoAtual);
      itFind->second.insert(listaEstados.begin(), listaEstados.end());
      return listaEstados;
}

//------------------------------------------------------------------------------
void DFA::CriaEstadoDeErro()
{
    this->estadoErro = new DFAEstado(-1);

    for(int i = 0; i < (int)this->Alfabeto.size(); i++)
        this->estadoErro->transicoes.emplace(this->Alfabeto[i], this->estadoErro);
}

//------------------------------------------------------------------------------
void DFA::renomeiaEstados()
{
    int i = 0;

    for(auto it = this->dfa.begin(); it != this->dfa.end(); ++it)
    {
        (*it)->id = i;
        i++;
    }
}
//------------------------------------------------------------------------------

NFAEstado* DFA::moveFecho(NFAEstado* estadoAtual, uchar simbolo)
{
    auto itFind = this->fechos.find(estadoAtual);
    NFAEstado* novoEstado = NULL;

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


//---------------------------------------------------------------------------------

void DFA::criarDFA()
{

    bool estadoExiste;
    bool primeiroErro = true;

    this->Alfabeto.pop_back();//Remove o EPSILON do alfabeto

    //Inserir estados na lista de &-fechos
    for(auto it = this->nfa.begin(); it != this->nfa.end(); ++it)
    {
        this->fechos.insert(std::make_pair(*it, std::unordered_set<NFAEstado*>({})));
    }

    //Cria a lista de fechos
    for(auto it = this->nfa.begin(); it != this->nfa.end(); ++it)
    {
        calculaFecho(*(it));

        //torna os estados não visitados para calcular o próximo fecho
        for(auto it2 = this->nfa.begin(); it2 != this->nfa.end(); ++it2)
        {
            (*it2)->fecho = false;
        }
    }

    //Cria o estado de erro (mesmo que não seja necessário)
    CriaEstadoDeErro();

    // inicia a pilha com estado inicial
    auto it = fechos.begin();
    std::deque<NFAEstado*> listaDeEstadosNFA;
    listaDeEstadosNFA.push_front(it->first);

    // Cria e insere o estado inicial (sem as transições)
    DFAEstado* novoEstadoDFA = new DFAEstado(it->first->id);

    for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        if((*it2)->estadoFinal)
            novoEstadoDFA->estadoFinal = true;

    this->dfa.push_front(novoEstadoDFA);


    //Parte que cria o DFA
    while(!listaDeEstadosNFA.empty())
    {

//        for(auto itaux = listaDeEstadosNFA.begin(); itaux != listaDeEstadosNFA.end(); ++itaux)
//            cout << (*itaux)->id << endl;

//        cout << endl;

        DFAEstado* atual;

        //busca o elemento atual do DFA para inserir transições
        for(auto it = this->dfa.begin(); it != dfa.end(); ++it)
            if((*it)->id == listaDeEstadosNFA.front()->id)
                atual = *it;

        for(int i = 0; i < (int)this->Alfabeto.size(); i++)
        {
            estadoExiste = false;

            NFAEstado* novoEstadoNFA = moveFecho(listaDeEstadosNFA.front(), this->Alfabeto[i]);

            if(novoEstadoNFA != NULL)
            {
                for(auto it = this->dfa.begin(); it != dfa.end(); ++it)
                    if((*it)->id == novoEstadoNFA->id)
                        estadoExiste = true;
                if(!estadoExiste)
                {
                    novoEstadoDFA = new DFAEstado(novoEstadoNFA->id);
                    this->dfa.push_back(novoEstadoDFA); //Insere um novo estado


                    //Insere a transição no estado atual
                    atual->transicoes.emplace(this->Alfabeto[i], novoEstadoDFA);

                    auto itFind = this->fechos.find(novoEstadoNFA);

                    for(auto it2 = itFind->second.begin(); it2 != itFind->second.end(); ++it2)
                        if((*it2)->estadoFinal)
                            novoEstadoDFA->estadoFinal = true;

                    listaDeEstadosNFA.push_back(novoEstadoNFA);

                }
                else
                {
                    for(auto it = dfa.begin(); it != dfa.end(); ++it)
                        if((*it)->id == novoEstadoNFA->id)
                             atual->transicoes.emplace(this->Alfabeto[i], *it);
                }
            }
            else
            {
                atual->transicoes.emplace(this->Alfabeto[i], this->estadoErro);

                if(primeiroErro)
                {
                    this->dfa.push_back(this->estadoErro);
                    primeiroErro = false;
                }
            }

        }

        //cout << "AA"<< endl;

        listaDeEstadosNFA.pop_front();
    }

    renomeiaEstados();
}

using namespace std;

//-----------------------------------------------------------------------------------------------
void DFA::show()
{
    for(auto it = Alfabeto.begin(); it !=Alfabeto.end(); ++it)
        cout<< " "<< *it;
    cout << endl;

    cout << nfa.size() << endl;

    for(auto it = dfa.begin(); it != dfa.end(); ++it) //percorre o deque
    {
        cout << "q" << (*it)->id << " : ";

        if((*it)->estadoFinal)
            cout << "Final";

        cout << endl;

        for(auto it2 = (*it)->transicoes.begin(); it2 != (*it)->transicoes.end(); ++it2)  /// percorre o unordered_multimap
            cout << (*it2).first << " :q" << (*it2).second->id << endl;

        cout << endl << endl;
    }
}
