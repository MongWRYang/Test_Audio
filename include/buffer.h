#include<stdio.h>
#include<stdbool.h>
#include<windows.h>
#define BUFQSIZE 48000*2*30

typedef struct{
    int size;
    BYTE buffer[BUFQSIZE];
    BYTE *head;
    BYTE *tail;
}BufferQueue;

BufferQueue* initBufferQueue(){
    BufferQueue* q = (BufferQueue*)malloc(sizeof(BufferQueue));
    memset(q->buffer,0,BUFQSIZE);
    q->size = 0;
    q->head = &q->buffer[0];
    q->tail = &q->buffer[1];
}

void PrintBufferQ(BufferQueue* q){
    printf("==========================\n");
    /*
    printf("all buf=");
    for(int i=0 ; i<BUFQSIZE ; i++){
        printf("%d ",q->buffer[i]);
    }
    printf(" ;\n");
    printf("buf=");
    for(int i=0 ; i<q->size ; i++){
        printf("%d ",*(q->head+i));
    }
    printf(" ;\n");
    */
    printf("f_add=%d, t_add=%d ; \n",q->head,q->tail);
    printf("buffer[0]add=%d, buffer[%d]=%d ; \n",&q->buffer[0],BUFQSIZE-1,&q->buffer[BUFQSIZE-1]);
    printf("bufsize=%d;\n",q->size);
    printf("==========================\n");
}

bool EnBufferQueue(BufferQueue* q,BYTE* d,int N){
    if(q->size+N > BUFQSIZE-1){
        printf("size:%d too big for BufferQueue \n",q->size+N);
        return false;
    }
    if(q->tail+N > &q->buffer[BUFQSIZE]){
        printf("not enough space for BufferQueue,only %d space left \n",&q->buffer[BUFQSIZE] - q->tail);
        return false;
    } 
    int i=0;
    while(i<N){
        *(q->tail-1) = *(d+i);
        q->tail = q->tail + 1; 
        i++;
    }
    q->size = q->size+N;
    return true;
}

bool DeBufferQueue(BufferQueue* q,BYTE* d,int N){
    if(q->size-N >= 0){
        q->size = q->size-N;
    }else{
        printf("not that much stuff to dequeue,only %d element left \n",q->size+N);
        return false;
    }
    int i=0;
    while(i<N){
        *(d+i) = *(q->head);
        *(q->head) = 0;
        q->head = q->head +1;
        i++;
    }
    return true;
}

bool ResetBufferQueue(BufferQueue* q){
    if(q->head > &q->buffer[0]){
        int i = 0;
        while(i < q->size){
            q->buffer[i] = *(q->head + i);
            *(q->head + i) = 0;
            i++;
        }
        q->head = &q->buffer[0];
        q->tail = q->head + q->size;
    }
}

void printBArr(BYTE* BA,int N){
    printf("barr= ");
    for(int i=0;i<N;i++){
        printf("%d ",BA[i]);
    }
    printf("; \n");
}