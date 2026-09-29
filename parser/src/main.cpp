#include <iostream>
#include "parser.h"

int main() {
    Parser parser;
    std::string filename = "compiladores.txt"; 

    std::cout << "Cargando gramática desde: " << filename << " ...\n" << std::endl;

    if (parser.loadGrammar(filename)) {
        parser.printGrammar();

        // 1. Calcular FIRST de todos los símbolos por punto fijo
        parser.computeFirstSets();
        parser.printFirstSets();

        std::cout << "\n==============================================" << std::endl;
        std::cout << "         PRUEBAS DE FIRSTCADENA (beta)        " << std::endl;
        std::cout << "==============================================" << std::endl;

        // Prueba 1: beta = { "E'", "T" }
        // Como E' es anulable (contiene epsilon), FIRST(E' T) incluye (FIRST(E') - {epsilon}) U FIRST(T)
        std::vector<std::string> beta1 = {"E'", "T"};
        std::set<std::string> res1 = parser.computeFirstOfString(beta1);

        std::cout << "\n1. FIRST( E' T ) = { ";
        for (auto it = res1.begin(); it != res1.end(); ++it) {
            std::cout << *it << (std::next(it) != res1.end() ? ", " : "");
        }
        std::cout << " }" << std::endl;

        // Prueba 2: beta = { "E'", "T'" }
        // Tanto E' como T' son anulables (producen epsilon). 
        // El resultado debe incluir sus terminales Y ADEMÁS 'epsilon' porque toda la cadena es anulable.
        std::vector<std::string> beta2 = {"E'", "T'"};
        std::set<std::string> res2 = parser.computeFirstOfString(beta2);

        std::cout << "2. FIRST( E' T' ) = { ";
        for (auto it = res2.begin(); it != res2.end(); ++it) {
            std::cout << *it << (std::next(it) != res2.end() ? ", " : "");
        }
        std::cout << " }" << std::endl;

        // Prueba 3: beta = { "T'", "$" }
        // T' es anulable, por lo que pasa al siguiente símbolo '$' (lookahead / EOF)
        std::vector<std::string> beta3 = {"T'", "$"};
        std::set<std::string> res3 = parser.computeFirstOfString(beta3);

        std::cout << "3. FIRST( T' $ ) = { ";
        for (auto it = res3.begin(); it != res3.end(); ++it) {
            std::cout << *it << (std::next(it) != res3.end() ? ", " : "");
        }
        std::cout << " }" << std::endl;


    } else {
        std::cerr << "Error al abrir el archivo de gramática." << std::endl;
    }

    return 0;
}