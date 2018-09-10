#include <stdio.h>
#include <audio_tools.h>

int main(int argc, char *argv[])
{
    aud16bi* aud_mic = aud16bi_alloc(16000, 4, 16*22);
    aud16bi* aud_spk = aud16bi_alloc(16000, 2, 16*22);
    return 0;
}

