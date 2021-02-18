#include <pthread.h>
#include "mandelbrot.h"
#include <cstdlib>	// malloc

unsigned rem;	// remain threads, initial value = num_threads

// Mandelbrot convergence map
unsigned mandelbrot[resolution*resolution];
// Lock and cond variables for thread_exit() and thread_join()
pthread_mutex_t lock;
pthread_cond_t  cond;

// This must be called at the end of thread function
void thread_exit(void) {
	//lock before access critical section
	pthread_mutex_lock(&lock);
	//critical section
	rem--;
	//wake up main thread
	if(rem==0)
		pthread_cond_signal(&cond);
	//unlock every end of cirtical section
	pthread_mutex_unlock(&lock);
	return;
}

// This is called by the main thread.
void thread_join(void) {
	//lock for main thread
	pthread_mutex_lock(&lock);	
	//if remain thread exists, unlock and go to sleep
	while(rem != 0)
		pthread_cond_wait(&cond, &lock);
	//unlock for main thread
	pthread_mutex_unlock(&lock);
}

// Thread function to calculate a part of Mandelbrot set.
void* thread_mandelbrot(void* arg) {
	//thread id
	unsigned tid = *(unsigned*)arg;
	//loop for each thread, start row is tid and step size equals num_threads
	for(unsigned h = tid ; h < resolution ; h = h + num_threads){
                for(unsigned w = 0 ; w <resolution; w++){
                        //possible complex number for mandelbrot set
                        Complex c(minW+(maxW-minW)*w/resolution, minH+(maxH-minH)*h/resolution);
                        //initial Z value
                        Complex z = c;
			//iteration count
                        unsigned i;
                        for(i = 0 ; i <max_iterations; i++){
				//mandelbrot calculation
                            	z = z * z + c;
				//compare (absolute value)^2 for z to escape constant
				if(z.magnitude2()>escape)
                                        break;
  		
  			}
                        //store the number of iteration
                        mandelbrot[w + h *resolution ] =i;
                }

        }
    thread_exit();  // Wake up a thread waiting on the condition variable.
    return 0;
}

// Calculate the Mandelbrot convergence map.
void calc_mandelbrot(void) {
	//threads pointer	
    	pthread_t *threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    	//threads ID pointer
	unsigned *tids = (unsigned*)malloc(num_threads * sizeof(unsigned));
    	rem = num_threads;	//initialize remain threads
	//create threads, input argument is its tid
    	for(unsigned i = 0 ; i < num_threads ; i++){
	    tids[i] = i;
	    pthread_create(&threads[i], NULL, &thread_mandelbrot, &tids[i]);
    	}
	//call newly defined thread joing function.
    	thread_join();
}
