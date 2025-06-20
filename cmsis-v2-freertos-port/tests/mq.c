/*
 * mq.c
 *
 *  Created on: Jun 7, 2025
 *      Author: erdem
 */

 #include "porting_layer.h"

 #if ACTIVATE_MQ_TEST
 
 no_task_handle_t tasks_handle[2];
 no_mq_t mq;
 
 DECLARE_TIME_COUNTERS(no_time_t, s_to_r);
 DECLARE_TIME_COUNTERS(no_time_t, r_to_s);
 
 no_task_retval_t mq_initialize_test(no_task_argument_t args)
 {
     // Create resources
     no_mq_create(&mq, 2, sizeof(int32_t));
 
     tasks_handle[0] = no_create_task(sender,
             "S",
             NO_DECREASE_TASK_PRIO(BASE_PRIO, 1) // sender is the low priority task.
         );
 
     tasks_handle[1] = no_create_task(receiver,
             "R",
             BASE_PRIO // receiver is the high priority task.
         );
 
     no_task_suspend_self();
     return TASK_DEFAULT_RETURN;
 }
 
 no_task_retval_t sender(no_task_argument_t args)
 {
     int32_t i;
 
     // 2b - Benchmark.
     for (i = 0; i < NB_ITER + 1; i++)
     {
         no_mq_send(&mq, 1);
         WRITE_T2_COUNTER(r_to_s);
     }
 
     for (i = 0; i < NB_ITER; i++)
     {
         WRITE_T1_COUNTER(s_to_r)
         no_mq_send(&mq, 1);
     }
 
     no_task_suspend_self();
 
     return TASK_DEFAULT_RETURN;
 }
 
 no_task_retval_t receiver(no_task_argument_t args)
 {
     int32_t i;
     DECLARE_TIME_STATS(int64_t)
 
     // 1 - Let sender start
     no_mq_receive(&mq);
 
     // 2a - Benchmark.
     for (i = 0; i < NB_ITER; i++)
     {
         WRITE_T1_COUNTER(r_to_s)
         no_mq_receive(&mq);
         COMPUTE_TIME_STATS(r_to_s, i);
 #ifndef TRACING
         no_cycle_reset_counter();
 #endif
     }
 
     REPORT_BENCHMARK_RESULTS("-- MQ: Receive block --");
     RESET_TIME_STATS();
 
     for (i = 0; i < NB_ITER; i++)
     {
         no_mq_receive(&mq);
         WRITE_T2_COUNTER(s_to_r)
         COMPUTE_TIME_STATS(s_to_r, i)
 #ifndef TRACING
         no_cycle_reset_counter();
 #endif
     }
 
     REPORT_BENCHMARK_RESULTS("-- MQ: Signal unblock --");
 
     no_task_suspend_self();
 
     return TASK_DEFAULT_RETURN;
 }
 
 #endif
 