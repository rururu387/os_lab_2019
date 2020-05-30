#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct factorialArgs
{
    int num;
    int mod;
    int curTNum;
    int tNum;
    int k;
};

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void factorial(void* _arg)
{
    struct factorialArgs* arg = _arg;
    for (int i = arg->curTNum; i <= arg->k; i += arg->tNum)
    {
        pthread_mutex_lock(&mut);
        arg->num = (arg->num * i) % arg->mod;
        pthread_mutex_unlock(&mut);
    }
}

int main(int argc, char **argv)
{
    int tNum = 1, mod = 1, k = 1;
    
    static struct option options[] = {
                                    {"k", required_argument, 0, 0},
                                    {"tNum", required_argument, 0, 0},
									{"mod", required_argument, 0, 0},
                                    {"h", no_argument, 0, 0},
									{0, 0, 0, 0}
                                    };

    while (1)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "k", options, &option_index);
		
		if(c == -1)
			break;
		
		switch(c)
		{
			case 0:
			{
				switch(option_index)
				{
                    case 0:
                    {
                        k = atoi(optarg);
                        if (k < 0)
                        {
                            printf("K must be a positive number or 0. Now K is %d\n", k);
                        }
                    }
					case 1:
					{
						tNum = atoi(optarg);
						if (tNum < 1)
						{
								printf("tNum must be a positive number. Now tNum is %d\n", tNum);
								return -1;
						}
						break;
					}
		
					case 2:
                    {
						mod = atoi(optarg);
						if (mod < 0)
						{
								printf("Mod must be a positive number. Now mod is %d\n", mod);
								return -1;
						}
						break;
                    }
                    case 3:
                    {
                        printf("Use: ./parallelFactorialMod --k [num] --tNum [num] --mod [mod]\n");
                        return 0;
                    }
				}
				break;
			}
			case '?':
				break;
		
			default:
				printf("getopt returned character code 0%o?\n", c);
		}
	}

    int curTNum = 0;
    pthread_t* thArray = (pthread_t*)malloc(sizeof(pthread_t) * curTNum);
    struct factorialArgs* arg = (struct factorialArgs*)malloc(sizeof(struct factorialArgs));
    arg->num = 1;
    arg->mod = mod;
    arg->tNum = tNum;
    arg->k = k;
    for (curTNum = 0; curTNum < tNum; curTNum++)
    {
        arg->curTNum = curTNum + 1;
        if (pthread_create(thArray + curTNum, NULL, (void*)factorial, (void*)arg) != 0)
        {
                perror("pthread_create");
                exit(1);
        }
    }

    curTNum--;

    for (; curTNum >= 0; curTNum--)
    {
        if (pthread_join(thArray[curTNum], NULL) != 0)
        {
            perror("pthread_join");
            exit(1);
        }
    }

    printf ("Answer: %d", arg->num);
    free(thArray);

    return 0;
}