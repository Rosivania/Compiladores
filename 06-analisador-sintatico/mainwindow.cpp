#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------------------
std::vector<std::string> MainWindow::tokenizar(std::string str, char *delim)
{
    char *aux = strdup(str.c_str());
    char *pch;
    std::vector<std::string> strTokenizada;

    pch = strtok(aux, delim);

    while(pch != NULL)
    {
        strTokenizada.push_back(pch);
        pch = strtok(NULL, delim);
    }

    return strTokenizada;
}
//---------------------------------------------------------------------------------------
void MainWindow::on_inserirGramatica_clicked()
{
    std::string strV, strT, strP, strS;

    std::vector<std::string> Vaux; // Conjunto de variáveis
    std::vector<std::string> Taux; // Conjunto de terminais
    std::vector<std::string> Paux; // Conjunto de produções

    std::string aux; // Auxilia na leitura das produções

    strV = ui->variaveis->text().toStdString();
    strT = ui->terminais->text().toStdString();
    strS = ui->inicial->text().toStdString();
    strP = ui->producoes->toPlainText().toStdString();

    Vaux = tokenizar(strV, " ,");
    Taux = tokenizar(strT, " ,");

    std::stringstream ss(strP);

    while(std::getline(ss, aux))
    {
        if(aux[aux.size()-1] == ',')
            aux.pop_back();

        Paux.push_back(aux);
    }

    ll::analisadorSintatico a(Vaux, Taux, Paux, strS);


    for(auto it = a.tabelaSintatica.begin(); it != a.tabelaSintatica.end(); ++it)
    {
              for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
              {
                  std::cout << it->first << ": ";

                  std::cout << it2->first << " ";
                  for(unsigned int i = 0; i < it2->second.size(); i++)
                    std::cout << it2->second[i] << " ";

                    std::cout << std::endl << std::endl;
              }

    }


}

