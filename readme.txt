PUC-Rio
2015.1
INF1715 Compiladores
Professor Roberto Ierusalimschy
Gabriel de Quadros Ligneul 1212560

Monga Compiler

Compilação e Utilização
    Para compilar o compilador deve-se chamar o programa make na raiz do
    projeto.

    Para rodar um programa monga, pode-se utilizar o script monga, que recebe
    como parâmetro um arquivo monga, o compila e imprime o resultado de
    execução. Exemplo:
    ./monga examples/sort.mng

Benchmark
    Comparações com ggc -o0:

Diretórios
    benchmark   Comparação com gcc -o0
    bin         Executáveis gerados
    build       Makefiles auxiliares
    examples    Exemplos de programas monga
    lib         Bibliotecas de monga
    obj         Objetos e arquivos de dependêcias
    src         Fontes
    tests       Testes

Fontes
    ast/
        ast.c           Estrutura de dados que representa a árvore abstrata
        ast_print.c     Imprime a árvore sintática
        type.c          Estrutura de dados que representa um tipo
    backend/
        assembly.c      Escreve o código assembly com base na ast
        locations.c     Calcula o offset da varíaveis
    parser/
        parser.tab.c    Parser gerado pelo yacc a partir do parser.y
    scanner/
        scanner.c       Scanner gerado pelo lex a partir do scanner.l
    semantic/
        symbols.c       Estrutura de dados que representa a tabela de símbolos
        semantic.c      Realiza análize semântica na ast
    util/
        error.c         Realiza tratamento de erro
        new.c           Macro útil para alocação dinâmica de memória
        table.c         Estrutura de dados que implementa uma árvore rubro negra
        vector.c        Estrutura de dados que implementa um vector

Testes
    Para cada componente, existe uma subpasta com seu nome dentro da pasta
    tests.

    Dentro de cada subpasta, existe uma série de duplas <%.in, %.exp>, onde:
    % é o nome do teste, %.in é o arquivo de entrada e %.exp é o arquivo
    esperado.

    Para executar os testes, deve-se executar o script run_tests.sh na raiz do
    projeto.

    Este script irá fazer a chamada "bin/$componente_tests <
    tests/$componente/$teste.in > tests/$componente/$teste.out" e então irá
    comparar o arquivo .out com o arquivo .exp.

    Para cada componente, caso todos os testes passem, será impressa a mensage
    "Test succeeded: $componente".

    Caso algum teste falhe, será feito um diff entre o arquivo esperado e o
    arquivo de saída.

Scanner
    Os únicos escapes aceitos são: \t, \n, \\ e \". Caso o programador tente
    compilar um programa que contenha um escape diferente (eg. \a), será
    impressa uma mensagem de erro e o programa não será aceito.

    A linguagem aceita caracteres literais, no formato '.', e também no
    formato '{escape}'. O tratamento para escapes em strings e em literais é
    o mesmo.

    Além das palavras reservadas especificadas no enúnciado, foram adicionadas:
    - 'null', que é a única representação da um ponteiro nulo.
    - 'delete' que é utilizada na liberação de memória alocada dinamicamente.
    - 'bool' que é utilizado na declaração do tipo booleano.
    - 'true' e 'false' que são literais do tipo booleano.

Parser
    Além do específicado, a linguagem aceita:
    - Protótipos de funções. Diferente de protótipos de C, estes devem
      apenas ser utilizados para funções externas.
    - A Expressão 'null', que representa um ponteiro nulo e é utilizado em
      comparações e atribuições.
    - O comando delete, que é utilizado para liberar memória alocada
      dinâmicamente. Este comando possui a regra: cmd -> 'delete' exp ';'.
    - O tipo básico bool, que é utilizado em expressões lógicas.

Ast
    A árvore abstrata está dividida em quatro tipos de nós: decl, cmd, exp e
    var. Para cada nó existe uma struct de unions de structs. E para cada
    struct interna existe uma tag diferente.

    Os nós decl, cmd e exp possuem os campos next e last. Estes servem para
    representar uma lista encadeada. Nesta lista, o campo next aponta para o
    próximo elemento e o campo last aponta para o último elemento. Contudo,
    apenas o primeiro elemento da lista apontará para o último. Isso ocorre
    porque este é um campo auxiliar para concatenar duas listas em tempo    
    constante.

Semantic
    A tabela de símbolos está encapsulada no módulo symbols, que trata
    internamente os casos de erro quando símbolos utilizados não estão
    declarados ou quando são redeclarados. Protótipos são casos especiais e
    podem ser redeclarados, contanto que possuam o mesmo tipo do anterior.

    Escopo funciona como C.

    A tipagem é feita dando enfoque à "type safety":
    - Expressões aritméticas { +, -, *, / } só podem ser feitas com combinações
      de inteiros e floats.
    - Em operações entre inteiros e floats, as expressões do tipo inteiro são
      convertidas para float.
    - Para qualquer combinação de inteiros e floats pode-se fazer qualquer uma
      das comparações: ==, !=, <, <=, >=, >. 
    - Para outras expressões, as únicas possíveis comparações são == e !=. Além
      disso, as subexpressões destas comparações devem ser do mesmo tipo, ou
      então uma delas deve ser do tipo array e outra do tipo null.
    - Todas as comparações resultam no tipo bool.
    - Expressões lógicas { &&, ||, ! } só podem ser feitas com expressões do
      tipo bool e resultam no tipo bool.
    - Os comandos if e while só podem conter expressões do tipo bool.
    - O comando delete só pode conter expressões do tipo array.
    - Em atribuições, as expressões devem ter o mesmo tipo da variável, ou devem
      ser reduzíveis ao tipo da variável. As possíveis reduções são:
        b. Int <- Float
        c. Float <- Int
        d. Char <- Float
        e. Char <- Int
        a. Type[] <- Null
    - A passagem de parâmetros segue a mesma regra das atribuições.

Backend
    O executável do backend se chama mc e está na basta bin. Este lê da entrada
    padrão um programa monga e escreve na saída padrão o código em assembly
    referente a este.

    Para facilitar a compilação e execução de programas monga, foi criado um
    script (./monga) que é responsável por todas as etapas de compilação: pré
    processamento, compilação para assembly, compilação para binário e linkagem
    com a lib de IO.

    Para ver os resultados intermediários deste processo, basta passar a flag
    -t para o script. Exemplo:
    ./monga examples/sort.mng -t
