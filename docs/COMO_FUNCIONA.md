# Como funciona

O módulo cria `/proc/proc_counter` com permissão `0644`: qualquer usuário pode
ler e o proprietário pode escrever. O kernel mantém o valor inteiro e os dois
contadores, protegidos pelo mesmo mutex.

## Leitura

`single_open` e `seq_file` produzem uma fotografia do estado. `seq_read` entrega
somente o número de bytes solicitado e mantém a posição entre leituras parciais.
Assim, ler um byte por chamada também entrega uma mensagem completa e termina
em EOF. O contador de leitura mede fotografias produzidas, não cada chamada
`read()` feita pelo programa.

## Escrita

Uma escrita não vazia aceita `reset` ou um inteiro decimal no intervalo de `int`.
Espaços nas extremidades são removidos. Texto inválido, NUL embutido e entradas
de 128 bytes ou mais são recusados antes de modificar o estado. Uma escrita
vazia retorna zero e preserva o estado.

Um inteiro aceito incrementa o contador de escritas. `reset` zera os contadores
e o valor. Cada acesso usa o mutex somente enquanto consulta ou altera o estado.

`proc_counter.c` é a implementação canônica. O antigo arquivo duplicado em
português foi substituído por este guia para evitar versões divergentes.

## Verificação

`make` compila o módulo contra os headers escolhidos por `KDIR`. O roteiro
`tests/check_counter.py` verifica o comportamento em um módulo já carregado e
altera seu estado de demonstração. Compilar o módulo não executa esse roteiro
nem comprova carregamento no kernel.
