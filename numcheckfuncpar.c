/* Você deve implementar uma versão usando paralelismo de funções (também
 * chamado decomposição por funções (function decomposition)
 * Para simplificar, você pode considerar que seu programa criará as
 * threads que você julgar necessárias para processar todos os números,
 * onde cada thread executará uma função diferente, com parte das ações
 * esperadas do programa final.
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
     have_four_repetitions = 0,
     all = 0;

long *track_all;


// Structures needed for parallelization
pthread_mutex_t lock;
struct thread_args {
  long maxnum;
  int ndigits;
};
// END Structures needed for parallelization

/* check_num: concentra todos os testes a serem aplicados a cada número.
 */
void *check_pal(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;
  long i;
  long maxnum = args->maxnum; 
  int ndigits = args->ndigits;
  int pal;

  digit_t num;

  for (i=0;i<=maxnum;++i) {
    break_into_digits(i, num, ndigits);
    pal = is_palindrome( num, ndigits );

    pthread_mutex_lock(&lock);
    palindromes += pal;
    track_all[i] += pal;
    if (pal == 1) {
      update_max( i, track_all[i] );

      if(track_all[i] == 1){
        // printf("[log] Number %d matched pal\n", i);
        match_some_test += 1;
      }
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void *check_rep(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;
  long i;
  long maxnum = args->maxnum; 
  int ndigits = args->ndigits;
  int rep;
  digit_t num;

  for (i=0;i<=maxnum;++i) {
    break_into_digits(i, num, ndigits);
    rep = has_repeated_seq( num, ndigits );

    pthread_mutex_lock(&lock);

    repeated_seqs += rep;
    track_all[i] += rep;
    if (rep == 1){
      update_max( i, track_all[i] );
      if(track_all[i] == 1 && rep == 1){
        // printf("[log] Number %d matched rep\n", i);
        match_some_test += 1;
      }
    }


    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void *check_sum(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;
  long i;
  long maxnum = args->maxnum; 
  int ndigits = args->ndigits;
  int sum;
  digit_t num;

  for (i=0;i<=maxnum;++i) {
    break_into_digits(i, num, ndigits);
    sum = sum_is_ap( num, ndigits );

    pthread_mutex_lock(&lock);

    sums_are_ap += sum;
    track_all[i] += sum;
    if(sum == 1){
      update_max( i, track_all[i] );
      if(track_all[i] == 1 && sum == 1){
        // printf("[log] Number %d matched sum\n", i);
        match_some_test += 1;
      }
    }


    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void *check_dou(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;
  long i;
  long maxnum = args->maxnum; 
  int ndigits = args->ndigits;
  int dou;
  digit_t num;

  for (i=0;i<=maxnum;++i) {
    break_into_digits(i, num, ndigits);
    dou = has_tripled_digits( num, ndigits );

    pthread_mutex_lock(&lock);
    have_tripled_digits += dou;
    track_all[i] += dou;
    if(dou == 1){
      update_max( i, track_all[i] );
      if(track_all[i] == 1 && dou == 1){
        // printf("[log] Number %d matched dou\n", i);
        match_some_test += 1;
      }
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void *check_fou(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;
  long i;
  long maxnum = args->maxnum; 
  int ndigits = args->ndigits;
  int fou;
  digit_t num;

  for (i=0;i<=maxnum;++i) {
    break_into_digits(i, num, ndigits);
    fou = has_four_repetitions( num, ndigits );

    pthread_mutex_lock(&lock);
    have_four_repetitions += fou;
    track_all[i] += fou;
    if(fou == 1){
      update_max( i, track_all[i] );
      if(track_all[i] == 1 && fou == 1){
        // printf("[log] Number %d matched fou\n", i);
        match_some_test += 1;
      }
    }

    pthread_mutex_unlock(&lock);
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

    // Alterações para paralelização por função
    int thread_count = 5;
    pthread_t thread_handles[thread_count];
    struct thread_args args;
    pthread_mutex_init(&lock, NULL);

    args.maxnum = maxnum;
    args.ndigits = ndigits;

    track_all = (long*) calloc(maxnum+1, sizeof(long));

    pthread_create(&thread_handles[0], NULL, check_pal, &args);
    pthread_create(&thread_handles[1], NULL, check_rep, &args);
    pthread_create(&thread_handles[2], NULL, check_sum, &args);
    pthread_create(&thread_handles[3], NULL, check_dou, &args);
    pthread_create(&thread_handles[4], NULL, check_fou, &args);

    for (i = 0; i < thread_count; i++)
      pthread_join(thread_handles[i], NULL);

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
