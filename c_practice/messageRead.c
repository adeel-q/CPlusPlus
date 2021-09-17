#include <stdio.h>
#include <stdint.h>

#define SYNC_TOKEN  '$'
#define MAX_MSG_SIZE  7
#define MAX_BUFFER_SIZE  256

#define COLLECT_NEW 0
#define COLLECT_REMAINING 1

static int currentState = COLLECT_NEW;

int onCollectRemaining(uint8_t* bytesRemaining, uint8_t buffer[MAX_BUFFER_SIZE][MAX_MSG_SIZE], uint8_t* bufferIndex, uint8_t* write_index, uint8_t* data, uint8_t length );
int onCollectNew(uint8_t* bytesRemaining, uint8_t buffer[MAX_BUFFER_SIZE][MAX_MSG_SIZE], uint8_t* bufferIndex, uint8_t* write_index, uint8_t* data, uint8_t length );
int findSyncIndex(uint8_t* data, int start_index, uint8_t length);

uint8_t buffer[MAX_BUFFER_SIZE][MAX_MSG_SIZE]; // Circular buffer
static uint8_t bufferIndex = 0;

// Called on successful and should be in a thread
void onParse()
{
    // Read queue
    // Block / Sleep on empty
    // Process messages
    // Compute CRC
    // Sleep / Yield
}

// Called async or periodically
void onData(uint8_t* data, uint8_t length)
{
    static uint8_t bytesRemaining = 0;
    static uint8_t write_index = 0;
    
    switch (currentState)
    {
        case COLLECT_NEW:
        {
            currentState = onCollectNew(&bytesRemaining, buffer, &bufferIndex, &write_index, data, length);
            break;
        }
        case COLLECT_REMAINING:
        {
            currentState = onCollectRemaining(&bytesRemaining, buffer, &bufferIndex, &write_index, data, length);
            break;
        }
        default:
        {
            break; 
        }
    }
}


int onCollectRemaining(uint8_t* bytesRemaining, uint8_t buffer[MAX_BUFFER_SIZE][MAX_MSG_SIZE], uint8_t* bufferIndex, uint8_t* write_index, uint8_t* data, uint8_t length )
{
    int nextState = COLLECT_REMAINING;
    if (length > 0)
    {
        int i = 0;
        while ((*bytesRemaining) >= 0 && i < length)
        {
            buffer[*bufferIndex][*write_index] = data[i];
            (*bytesRemaining)--;
            (*write_index)++;
            i++;
        }
        if ((*bytesRemaining) == 0)
        {   
            (*write_index) = 0;
            nextState = onCollectNew(bytesRemaining, buffer, bufferIndex, write_index, data, length);
        }
    }
    else
    {
        // No bytes in this buffer. We still are waiting on the remaining bytes...
    }
    return nextState;
}

int onCollectNew(uint8_t* bytesRemaining, uint8_t buffer[MAX_BUFFER_SIZE][MAX_MSG_SIZE], uint8_t* bufferIndex, uint8_t* write_index, uint8_t* data, uint8_t length )
{
    int keepReading = 1;
    
    int nextState = COLLECT_NEW;

    int i = 0; // Point to the current data index
    
    while (keepReading)
    {
        // Find sync bytes between i and up to length
        i = findSyncIndex(data, i, length);
        if (i == -1)
        {
            // No start byte in here. Discard packet
            keepReading = 0;
        }
        else
        {
            // Read 7 bytes, move data ptr up
            // If reached length, set state to COLLECT_REMAINING, set reamining bytes to be read. return
            // If reached 7 bytes, find the next index and keep reading
            (*bytesRemaining) = MAX_MSG_SIZE;
            
            while ((i < length) && ((*bytesRemaining) >=0))
            {
                buffer[*bufferIndex][*write_index] = data[i];
                i++;
                (*write_index)++;
                (*bytesRemaining)--;
            }
            if (*bytesRemaining == 0)
            {
                (*bufferIndex)++; // Next buffer to fill
                (*write_index) = 0; // reset write index
                continue; // Go back and attempt to read the reamining buffer
            }
            else
            {
                // We await data from the next time this method is called
                nextState = COLLECT_REMAINING;
                keepReading = 0;
            }
        }
    }
    return nextState;
}

// Given start pivot point, find the next sync byte
int findSyncIndex(uint8_t* data, int start_index, uint8_t length)
{
    int ret = -1; // If not found , retuyrn -1
    int found = 0;
    if (data && length && (start_index < length) && (start_index >= 0))
    {
        for (int i = start_index; (i < length && !found); i++)
        {
            if (data[i] == SYNC_TOKEN)
            {
                ret = i;
                found = 1;
            }
        };
    }
    return ret;
}


int main()
{

    uint8_t input[20] = {(uint8_t)'$', 2,3,4,5,6,7, (uint8_t) '$', 8};
    

    onData(input, 9);
    printf("Buffers filled %i \n", bufferIndex);
    printf("State is %s \n", currentState ? "COLLECT_REMAINING" : "COLLECT_NEW");
    for (int i = 0; i < bufferIndex+1; i++)
    {
        printf("buffer[%i]= [\"", bufferIndex);
        for (int j = 0; j < MAX_MSG_SIZE; j++)
        {
            if (j == 0)
            {
                printf("%c", buffer[i][j]);
            }
            else
            {
                printf("%i", buffer[i][j]);
            }
        }
        printf("\"]\n");
    }

    return 0;
}