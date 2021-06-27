/* Você deve implementar uma versão usando paralelismo de dados (também
 * chamado decomposição/partição de domínio (domain partition/decomposition)
 * Para simplificar, você pode considerar que seu programa criará oito
 * threads para processar todos os números. Todas as threads devem executar
 * o mesmo código, já que estarão executando as mesmas operações para
 * partes diferentes do conjunto de dados.
 * 
 * O programa deve aceitar o mesmo parâmetro de linha de comando da versão
 * sequencial - e nenhum outro. 
 * A saída deve sequir exatamente o mesmo formato da versão sequencial.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "timediff.h"   // calcula tempo decorrido
#include "numchecks.h"  // conta números com mais condições válidas
#include "conditions.h" // verifica cada condição

// Biblioteca para criação de threads
#include <pthread.h>

// Contadores para cada uma das condições testadas
long match_some_test = 0,
     palindromes = 0,
     repeated_seqs = 0,
     sums_are_ap = 0,
     have_tripled_digits = 0,
     have_four_repetitions = 0;


// Structures needed for parallelization

pthread_mutex_t lock;

struct thread_args {
  int min, max, ndigits;
};
// END Structures needed for parallelization

/* check_num: concentra todos os testes a serem aplicados a cada número.
 */
void check_num(long n, int ndigits)
{
    int all, pal, rep, sum, dou, fou;
    digit_t num;
    long orign = n;

    // Transforma número (n) em vetor de dígitos (num)
    break_into_digits(n, num, ndigits);

    // Aplica os diversos testes a um dado número
    pal = is_palindrome( num, ndigits );
    rep = has_repeated_seq( num, ndigits );
    sum = sum_is_ap( num, ndigits );
    dou = has_tripled_digits( num, ndigits );
    fou = has_four_repetitions( num, ndigits );

    // Para processar número de condições satisfeitas
    all = pal + rep + sum + dou + fou;
    pthread_mutex_lock(&lock);
    // printf("[log] Entered critical region, n = %d\n", (int) n);

    if (all>0) {
        match_some_test += 1;
    }

    update_max( orign, all );

    // Atualiza os contadores por condição
    palindromes += pal;
    repeated_seqs += rep;
    sums_are_ap += sum;
    have_tripled_digits += dou;
    have_four_repetitions += fou;

    pthread_mutex_unlock(&lock);
}

void *call_checknum(void *arguments){
  struct thread_args *args = (struct thread_args *)arguments;
  int min, max, ndigits;
  min = args->min;
  max = args->max;
  ndigits = (int) args->ndigits;

  int i;
  printf("[log] Calling checknum with interval [%d,%d]\n", min, max);
  for(i = min; i <= max; i++){
    check_num(i,ndigits);
  }

  return NULL;
}


int main( int argc, char* argv[] ) {
    int  ndigits; // núm. de dígitos para representar até o maior número

    long i, tmp, maxnum;

    struct timeval t1, t2; // marcação do tempo de execução

    // tratamento da linha de comando
    if (argc!=2) {
        fprintf(stderr,"usage: %s maxnum\n",argv[0]);
        exit(1);
    }
    maxnum = atol(argv[1]);

    // determinação de ndigits em função do maxnum
    tmp = maxnum;
    ndigits=0;
    do {
        ndigits++;
        tmp=tmp/10;
    } while (tmp>0);


    // Marca o tempo e checa cada número na faixa definida.
    // Note que o valor do parâmetro maxnum é considerado inclusive (<=)
    gettimeofday(&t1,NULL);

    // Alterações para paralelização por domínio
    // Divide o domínio em 8 partes iguais e dispara uma thread para cada parte
    
    int thread_count = 8;
    struct thread_args args[thread_count];
    pthread_t thread_handles[thread_count];
    pthread_t odd_thread;
    pthread_mutex_init(&lock, NULL);

    int partition = (int) maxnum / thread_count;
    printf("[log]: partition = %d\n", partition);
    int min, max;

    int isOdd = maxnum % 2 != 0;

    if (isOdd){
      struct thread_args odd_args;
      odd_args.min = partition * thread_count + 1;
      odd_args.max = (int) maxnum;
      odd_args.ndigits = ndigits;
      printf("[log] Odd maxnum; min = %d max = %d\n", odd_args.min, odd_args.max);
      pthread_create(&odd_thread, NULL, call_checknum, (void*) &odd_args);
    }

    // printf("[log]: maxnum = %d\n", maxnum);
    for (i = 0; i < thread_count; i++){
      min =  i * partition + 1;
      max = min + partition - 1;
      // printf("[log] interval: [%d,%d]\n", min, max);

      if (i == 0) {
        min =  i * partition;
        max = min + partition;
      }


      args[i].min = min;
      args[i].max = max;
      args[i].ndigits = ndigits;


      pthread_create(&thread_handles[i], NULL, call_checknum, &args[i]);
    }

    if (isOdd){
      pthread_join(odd_thread, NULL);
    }

    for (i = 0; i < thread_count; i++){
      pthread_join(thread_handles[i], NULL);
    }

    gettimeofday(&t2,NULL);

    pthread_mutex_destroy(&lock);

    // Escrita das estatísticas ao final da execução
    printf("%ld match_some_test (%d%%)\n", match_some_test, (int)((100.0*match_some_test)/maxnum));
    printf("%ld palindromes\n", palindromes);
    printf("%ld repeated_seqs\n", repeated_seqs);
    printf("%ld sums_are_ap\n", sums_are_ap);
    printf("%ld have_tripled_digits\n", have_tripled_digits);
    printf("%ld have_four_repetitions\n", have_four_repetitions);
    print_max( ndigits );
    printf("\ntempo: %lf\n",timediff(&t2,&t1));
}
