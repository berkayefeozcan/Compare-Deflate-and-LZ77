#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define SS_SEARCHBUFFER 32000// arama tamponun boyutu.
#define SS_LOOKAHEADBUFFER 255
#define FLAGBITS 1
#define SS_OFFSETBITS 15
#define SS_OFFSETMASK ((1 << (SS_OFFSETBITS)) - 1)
#define SS_GETFLAG(x) (x >> SS_OFFSETBITS)
#define SS_GETOFFSET(x) (x & SS_OFFSETMASK)
#define SETFLAGANDOFFSET(x,y) ((x <<SS_OFFSETBITS) | y)
#define SEARCHBUFFER 500 // 5 bit yer ayrildi.
#define LOOKAHEADBUFFER 20
#define LENGTHBITS 3
#define OFFSETBITS 5
#define LENGTHMASK ((1 << (LENGTHBITS)) - 1)
#define GETOFFSET(x) (x >> LENGTHBITS)
#define GETLENGTH(x) (x & LENGTHMASK)
#define SETOFFSETANDLENGTH(x,y) ((x <<LENGTHBITS) | y) // offseti 3 birim sola kaydirip length ile orladik.

int *characterOrLengthFrequency= NULL; // karakterlerin frekanslarini tutacak huffman tablosunda
int *flagAndOffsetFrequency = NULL;// bayrak ve ofsetin frekansi huffman tablosunda
uint8_t * charactersHuffArray=NULL;// karakterleri tutacak huffman tablosuda
uint16_t * flagAndOffsetHuffArray=NULL;// bayrak ve karakterin codu huffman agacinda
struct huffmanTreeForCharacterOrLength * root = NULL; // agac baslangic olarak bos.
struct huffmanTreeForFlagAndOffset * rootFlagAndOffset = NULL;

struct huffmanCodeArrayStruct *huffmanCodingArrayForFlagAndOffset;
int huffmanCodingArrayForFlagAndOffsetSize=1;
char * huffmanElementsArray ;//=(char*) malloc(sizeof(char)) ; // daha sonra bulunacak.
int * huffmanElementsArrayForFlagAndOffset  ;
int sizeOfElementsArray =1;
int * huffmanCodingArray ;//= malloc(sizeof(int)) ; // integer olarak kodlanmis huffman codlarini tutacak arraylar.
int huffmanCoddingArraySize=1;
struct binaryTreeOffsetAndFlag *binaryTreeFAndORoot =NULL; // offset ve flag in huffman kodunu binary search e ekledik.
struct binaryTreeCharacteOrLength *binaryTreeCOLRoot =NULL;

struct lzSSToken
{
    uint16_t flagAndOffset; // bayrak ve nekadar uzaklikta eslesme olacagini tutacak.
    uint8_t lengthOrCharacter; // bayrak 1 se karakterin ascii karsiligini yazmak yerine uzunlugu yazacak.
};

struct token
{
    // bu yapi lz77 icin yapilmistir.
    uint8_t offset;
    uint8_t length; // eslesme desenin uzunlugu  eslesilen karakterden olan uzaklýk.
    // eger uzungugu ve offseti ayri ayri yapsaydik encode islemi masrafli olacakti.
    char character; // karakter
};
struct huffmanCodeArrayStruct
{
    // bu yapi 0 1 li code arraylarini tutacak.
    int *array;
};
struct binaryTreeElementForFlagAndOffset
{
    int *codeArray ;
    uint16_t element;

};
struct binaryTreeElementForCharacterAndLength
{
    int *codeArray ;
    uint8_t element;

};
struct huffmanTreeForCharacterOrLength  // nodeleri tutacak linked list.
{

    struct huffmanTreeForCharacterOrLength *next;
    struct nodeForCharacterOrLength *nodeForCharacterOrLength;

};
struct huffmanTreeForFlagAndOffset  // nodeleri tutacak linked list.
{

    struct huffmanTreeForFlagAndOffset *next;
    struct nodeForFlagAndOffset *nodeForFlagAndOffset;

};
struct nodeForFlagAndOffset
{

    uint16_t flagAndOffset;
    int frequency ;
    struct nodeForFlagAndOffset *right;
    struct nodeForFlagAndOffset *left;

};
struct nodeForCharacterOrLength
{

    uint8_t character;
    int frequency ;
    struct nodeForCharacterOrLength *right;
    struct nodeForCharacterOrLength *left;

};

struct binaryTreeOffsetAndFlag // offset ve flag icin yapilan binary tree
{
    struct binaryTreeElementForFlagAndOffset *element ; // kod ve kodlanan karakteri tutan struct
    struct binaryTreeOffsetAndFlag *left;
    struct binaryTreeOffsetAndFlag *right;
};
struct binaryTreeCharacteOrLength // offset ve flag icin yapilan binary tree
{
    struct binaryTreeElementForCharacterAndLength *element ; // kod ve kodlanan karakteri tutan struct
    struct binaryTreeCharacteOrLength *left;
    struct binaryTreeCharacteOrLength *right;
};
struct binaryTreeOffsetAndFlag * InsertOffsetAndFlag(struct binaryTreeOffsetAndFlag *node,struct binaryTreeElementForFlagAndOffset *element)
{
    if(node==NULL)
    {
        struct binaryTreeOffsetAndFlag *temp;
        temp = (struct binaryTreeOffsetAndFlag *)malloc(sizeof(struct binaryTreeOffsetAndFlag));
        temp -> element = element;
        temp -> left = temp -> right = NULL;
        return temp;
    }
    int value = (int)element->element;
    int valueNode =(int ) node->element->element;
    if( value > valueNode)
    {
        node->right = InsertOffsetAndFlag(node->right,element);
    }
    else if(value < valueNode)
    {
        node->left = InsertOffsetAndFlag(node->left,element);
    }
    /* Else there is nothing to do as the data is already in the tree. */
    return node;
}
void clrscr()
{
    system("@cls||clear");
}
struct binaryTreeCharacteOrLength * InsertCharacterOrLength(struct binaryTreeCharacteOrLength *node,struct binaryTreeElementForCharacterAndLength *element)
{
    if(node==NULL)
    {
        struct binaryTreeCharacteOrLength *temp;
        temp = (struct binaryTreeCharacteOrLength *)malloc(sizeof(struct binaryTreeCharacteOrLength));
        temp -> element = element;
        temp -> left = temp -> right = NULL;
        return temp;
    }
    int value = (int)element->element;
    int valueNode =(int ) node->element->element;
    if( value > valueNode)
    {
        node->right = InsertCharacterOrLength(node->right,element);
    }
    else if(value < valueNode)
    {
        node->left = InsertCharacterOrLength(node->left,element);
    }
    /* Else there is nothing to do as the data is already in the tree. */
    return node;
}

int* FindOffsetAndFlag(struct binaryTreeOffsetAndFlag *node, int data)
{

    if(node==NULL)
    {
        /* Element is not found */
        return NULL;
    }
    if(data > (int)node->element->element)
    {
        /* Search in the right sub tree. */
        return FindOffsetAndFlag(node->right,data);
    }
    else if(data< (int)node->element->element)
    {
        /* Search in the left sub tree. */
        return FindOffsetAndFlag(node->left,data);
    }
    else

    {
        return node->element->codeArray;

    }
}
int* FindCharacterOrLength(struct binaryTreeCharacteOrLength *node, int data)
{

    if(node==NULL)
    {
        /* Element is not found */
        return NULL;
    }
    if(data > (int)node->element->element)
    {
        /* Search in the right sub tree. */
        return FindCharacterOrLength(node->right,data);
    }
    else if(data< (int)node->element->element)
    {
        /* Search in the left sub tree. */
        return FindCharacterOrLength(node->left,data);
    }
    else

    {
        return node->element->codeArray;

    }
}

void PrintInorderOffsetAndFlag(FILE *f,struct binaryTreeOffsetAndFlag *node,char *fileName)
{
    if(node==NULL)
    {
        //printf("bos");
        return;
    }
    f = fopen(fileName,"a+");
    fseek(f,0,SEEK_END); // dosyanin sonuna gidiliyor.
    PrintInorderOffsetAndFlag(f,node->left,fileName);
    fwrite(node->element,1,sizeof(node->element),f); // dosyaya yazdiriliyor.
    fclose(f);
    PrintInorderOffsetAndFlag(f,node->right,fileName);
}
void PrintInorderCharacterOrLength(FILE *f,struct binaryTreeCharacteOrLength *node,char *fileName)
{
    if(node==NULL)
    {
        //printf("bos");
        return;
    }
    f = fopen(fileName,"a+");
    fseek(f,0,SEEK_END); // dosyanin sonuna gidiliyor.
    PrintInorderCharacterOrLength(f,node->left,fileName);
    fwrite(node->element,1,sizeof(node->element),f); // dosyaya yazdiriliyor.
    fclose(f);
    PrintInorderCharacterOrLength(f,node->right,fileName);
}

// fonksiyon arama tamponuve ileri tapondaki esleme uzunlugu bulunuyor.
//sadece bir tane eslesme vasra lenght 1 olur.
int  findLengthOfMatch(char *sb,char *lb,int limit)
{
    int len =0 ;
    while( *sb++ == *lb++ && len<limit)  // eslesmenin uzunlugunu buluyoruz.
    {
        len++;
    }
    return len; // bulunan eslesmenin uzunlugunu dondurur.
    free(sb);
    free(lb);

}
void LZ77writeToFile(struct token *array,char * fileName,int sizeOfTokenArray)
{
    FILE *f =fopen(fileName,"wb");
    if(f)  // binary yazma kipinde aciliyor.
    {
        fwrite(array,sizeof(struct token),sizeOfTokenArray,f);
        fclose(f);
    }
    else
        printf("%s dosyasina yazilimadi.",fileName);

}
// bu fonksiyon compress islemi yapacak.
void LZ77encode (char *text,int textLenght,char *fileName)
{

    int sizeOfTokenArray = 1;
    char *lookAHeadBuffer,*searchBuffer;// arama ve ileri tampon pointerlari olusturuldu.
    int maxLength=0,len;// bu degiskenler dolasim aninda kullanilacak.
    int offset ;
    struct token *encodeArray = malloc(sizeof(struct token)); // token arrayi icin bir yer aciyoruz.
    char *maxtMatchLocation ;//maxsimum eslesmenin basladigi yeri tutan token.

    // metinin basindan baslayarak sonuna gidiyoruz. bunu lookahead buffer ile yapacagiz.
    //bu dongu window u degistirecek.
    for(lookAHeadBuffer=text; lookAHeadBuffer < text + textLenght ; lookAHeadBuffer++)
    {
        struct token t ; //token arrayine eklencek token
        if(lookAHeadBuffer!=text)
            encodeArray = realloc(encodeArray,sizeof(struct token)*sizeOfTokenArray);
        // tokenler icin yer actik.
        searchBuffer = lookAHeadBuffer-SEARCHBUFFER; //arama tamponunu ileri tampondan arama tamponunun uzunlugu kadar gerisine koyduk.
        if(searchBuffer< text )
            searchBuffer = text; // arama tamponunun textin disina cikmasini engelliyoruz.
        //eger eslesme saglanmayacaksa varsayilan degerler atiliyor.


        // buffer search bufferin uzerinde gezinecek.
        maxLength = 0;
        maxtMatchLocation=searchBuffer;
        for(; searchBuffer<lookAHeadBuffer; searchBuffer++) // eslesen karakeri search buffer icinde ariyoruz..
        {


            len =findLengthOfMatch(searchBuffer,lookAHeadBuffer,LOOKAHEADBUFFER);// eslesmenin uzunlugunu buluyoruz.
            if(len>maxLength) // maxsimum eslemeye gore
            {
                maxLength=len ;
                maxtMatchLocation=searchBuffer ; // eger esleme maxsimum uzunluga gore baslangic yerini kaydediyoruz.
            }


        }
        if(lookAHeadBuffer+maxLength >=text+textLenght)
            maxLength = text + textLenght - lookAHeadBuffer - 1; // eger son karakter de tekrara dahilse text disina cikmamasin icin azalttik.

        t.character=*(lookAHeadBuffer+maxLength);
        if(maxLength>0)
            offset=lookAHeadBuffer-maxtMatchLocation; // eger eslesme uzunlugu sifirdan buyukse eslesen karakterden uzakligi hesaplandi.
        else
            offset =0 ;
        t.offset=offset;
        t.length=maxLength;
        encodeArray[sizeOfTokenArray-1]= t; // token arrayina pencereden buldugumuz tokeni atiyoruz.
        sizeOfTokenArray++;
        lookAHeadBuffer += maxLength; // ileri tamponu eslesen uzunluk kadar oteliyoruz.
    }
    LZ77writeToFile(encodeArray,fileName,(int)sizeOfTokenArray); // sonuc dosyaya yazdirilacak.
}


char* readSourceFile(char *fileName,int *textSize)
{
    FILE *f =fopen(fileName,"rb");
    char * text =0; // dondurelecek text
    if (f )
    {
        fseek(f,0,SEEK_END);
        *textSize = ftell(f)-1;
        fseek(f,0,SEEK_SET);
        text = malloc(*textSize); // metnin uzunlugu kadar texte boyut ayiriliyor.
        fread(text,1,*textSize,f);// dosya sonundaki son bir eleman alinmiyor.
        fclose(f); // dosya kapatiliyor.
    }
    else
        printf("%s dosyasi okunurken hata olustu.",fileName);

    return text;
}
int isLeafFlagAndOffset(struct nodeForFlagAndOffset* root) // dugumun yaprak olup olmadigina bakiyoruz.

{

    return (root->left==NULL) && (root->right==NULL );
}
int isLeafCharOrLength(struct nodeForCharacterOrLength* root) // dugumun yaprak olup olmadigina bakiyoruz.

{

    return (root->left==NULL) && (root->right==NULL );
}

void findCodeForLengthOrCharacter(struct nodeForCharacterOrLength* root, int arr[], int top)

{
    // huffaman kodu her element icin iteger olarak tutulacak

    // recursif olarak sol tarafa 0 atiyoruz.
    if (root->left)
    {

        arr[top]= 0;
        findCodeForLengthOrCharacter(root->left, arr, top + 1);
    }

    // recursif olarak sag tarafa 1 atiyoruz.
    if (root->right)
    {

        arr[top]=1;
        findCodeForLengthOrCharacter(root->right, arr, top + 1);
    }

    // eger yapraksa arrayi yazdiriyoruz.
    if (isLeafCharOrLength(root))
    {
        arr[top] = -1 ;
        struct binaryTreeElementForCharacterAndLength *node = malloc( sizeof(struct binaryTreeElementForCharacterAndLength));
        node->element = root->character;
        node->codeArray = malloc(sizeof(int)*(top+1));

        for(int i =0; i<top+1; i++)
        {
            node->codeArray[i]=arr[i];
        }

        //struct binaryTreeOffsetAndFlag *binaryNode =malloc(sizeof(struct binaryTreeOffsetAndFlag));
        binaryTreeCOLRoot = InsertCharacterOrLength(binaryTreeCOLRoot,node);


        top =0 ;
    }

}
void findCodeForFlagAndOffset ( struct nodeForFlagAndOffset* root, int arr[], int top,int arraySize)

{
    // huffaman kodu her element icin iteger olarak tutulacak

    // recursif olarak sol tarafa 0 atiyoruz.
    if (root->left)
    {

        arr[top]= 0;
        findCodeForFlagAndOffset(root->left, arr, top + 1,arraySize);
    }

    // recursif olarak sag tarafa 1 atiyoruz.
    if (root->right)
    {

        arr[top]=1;
        findCodeForFlagAndOffset(root->right, arr, top + 1,arraySize);
    }
    // eger yapraksa arrayi yazdiriyoruz.
    if (isLeafFlagAndOffset(root))
    {
        struct binaryTreeElementForFlagAndOffset *node = malloc( sizeof(struct binaryTreeElementForFlagAndOffset));
        node->element = root->flagAndOffset;
        node->codeArray= malloc(sizeof(int)*(top+1));
        arr[top] = -1 ;
        for(int i =0; i<top+1; i++)
        {
            node->codeArray[i]=arr[i];

        }


        // olusturulan huffman kodu huffman kodlarini tutan ar raya eklendi

        //struct binaryTreeOffsetAndFlag *binaryNode =malloc(sizeof(struct binaryTreeOffsetAndFlag));
        binaryTreeFAndORoot = InsertOffsetAndFlag(binaryTreeFAndORoot,node);

        top =0 ;
    }

}

void addNodeToHuffmanTreeForCharacterOrLength(uint8_t character,int frequency,struct nodeForCharacterOrLength *right, struct nodeForCharacterOrLength *left)
{
    // bu fonksiyon lzss algoritmasinin karakterlerini

    struct nodeForCharacterOrLength *node = (struct nodeForCharacterOrLength*) malloc(sizeof(struct nodeForCharacterOrLength));// yeni node icin bellekten yer ayirdik
    node->character=character;
    node->left=left;
    node->right = right;
    node->frequency = frequency;
    //struct huffmanTreeForCharacterOrLength *huffmanNode= malloc(sizeof(struct huffmanTreeForCharacterOrLength));
    struct huffmanTreeForCharacterOrLength *huffmanNode = malloc(sizeof(struct huffmanTreeForCharacterOrLength));

    if(root == NULL)
    {
        huffmanNode->nodeForCharacterOrLength=node;
        huffmanNode->next=NULL;
        root = huffmanNode;

    }
    else if ( root->nodeForCharacterOrLength->frequency > node->frequency ) // eger nodeden kucukse
    {

        huffmanNode->nodeForCharacterOrLength = node ;
        huffmanNode->next = root ;
        root = huffmanNode;
    }
    else
    {
        struct huffmanTreeForCharacterOrLength *iterator = root;
        // roottan baslayarak sirali ekleme yapiyoruz.

        for(; iterator->next != NULL && iterator->next->nodeForCharacterOrLength->frequency < node->frequency  ; iterator = iterator->next);

        huffmanNode->nodeForCharacterOrLength = node ;

        if(iterator->next == NULL )
        {

            iterator->next= huffmanNode;
            huffmanNode->next = NULL;
        }
        else
        {
            struct huffmanTreeForCharacterOrLength *temp ;
            temp=iterator->next;
            iterator->next = huffmanNode;
            huffmanNode->next = temp;
        }

    }

}
void addNodeToHuffmanTreeForFlagAndOffset(uint16_t flagAndOffset,int frequency,struct nodeForFlagAndOffset *right, struct nodeForFlagAndOffset *left)
{
    // bu fonksiyon lzss algoritmasinin karakterlerini

    struct nodeForFlagAndOffset *node = (struct nodeForFlagAndOffset*) malloc(sizeof(struct nodeForFlagAndOffset));// yeni node icin bellekten yer ayirdik
    node->flagAndOffset=flagAndOffset;
    node->left=left;
    node->right = right;
    node->frequency = frequency;
    //struct huffmanTreeForCharacterOrLength *huffmanNode= malloc(sizeof(struct huffmanTreeForCharacterOrLength));
    struct huffmanTreeForFlagAndOffset *huffmanNode = malloc(sizeof(struct huffmanTreeForFlagAndOffset));

    if(rootFlagAndOffset == NULL)
    {
        huffmanNode->nodeForFlagAndOffset=node;
        huffmanNode->next=NULL;
        rootFlagAndOffset = huffmanNode;

    }
    else if ( rootFlagAndOffset->nodeForFlagAndOffset->frequency > node->frequency ) // eger nodeden kucukse
    {

        huffmanNode->nodeForFlagAndOffset = node ;
        huffmanNode->next = rootFlagAndOffset ;
        rootFlagAndOffset = huffmanNode;
    }
    else
    {
        struct huffmanTreeForFlagAndOffset *iterator = rootFlagAndOffset;
        // roottan baslayarak sirali ekleme yapiyoruz.

        for(; iterator->next != NULL && iterator->next->nodeForFlagAndOffset->frequency < node->frequency  ; iterator = iterator->next);

        huffmanNode->nodeForFlagAndOffset = node ;

        if(iterator->next == NULL )
        {

            iterator->next= huffmanNode;
            huffmanNode->next = NULL;
        }
        else
        {
            struct huffmanTreeForFlagAndOffset *temp ;
            temp=iterator->next;
            iterator->next = huffmanNode;
            huffmanNode->next = temp;
        }

    }

}
void initHuffmanTreeForLengthOrCharacter(uint8_t * characters, int * frequenyArray,int sizeOfHuffArray)
{
    int i;
    //addNodeToHuffmanTree(characters[0],frequenyArray[0],NULL,NULL);
    for(i=0; i<sizeOfHuffArray; i++)
    {
        addNodeToHuffmanTreeForCharacterOrLength(characters[i],frequenyArray[i],NULL,NULL);
        //printf("%c-%d\n",characters[i],frequenyArray[i]);
    }

}
void initHuffmanTreeForFlagAndOffset(uint16_t * flagAndOffset, int * frequenyArray,int sizeOfHuffArray)
{
    int i;
    //addNodeToHuffmanTree(flagAndOffset[0],frequenyArray[0],NULL,NULL);
    for(i=0; i<sizeOfHuffArray; i++)
    {
        addNodeToHuffmanTreeForFlagAndOffset(flagAndOffset[i],frequenyArray[i],NULL,NULL);
        //printf("%c-%d\n",flagAndOffset[i],frequenyArray[i]);
    }

}
void printfHuffanTree()
{
    struct huffmanTreeForFlagAndOffset *iterator ;
    for(iterator = root ; iterator != NULL ; iterator = iterator->next)
    {
        printf("fre : %d - uint8_t : %c\n", iterator->nodeForFlagAndOffset->frequency,iterator->nodeForFlagAndOffset->flagAndOffset);

    }
}

void startHuffmanCoddingForCharOrLength(uint8_t * characters, int * frequencyArray,int sizeOfHuffArray)
{
    int len = sizeOfHuffArray;

    int i,sumFrequeny;
    struct huffmanTreeForCharacterOrLength *temp ;
    initHuffmanTreeForLengthOrCharacter(characters,frequencyArray,len);// karakterler ve frekanslari huffman agacina ekleniyor.

    for(i=0; i<len-1; i++)
    {
        sumFrequeny = root->nodeForCharacterOrLength->frequency + root->next->nodeForCharacterOrLength->frequency; // toplam frekanslar
        // karakterler disinde toplamlar oldugunda default olarak NULL degeri atanir.
        addNodeToHuffmanTreeForCharacterOrLength(0,sumFrequeny,root->next->nodeForCharacterOrLength,root->nodeForCharacterOrLength);
        temp = root->next->next;
        root=temp;
    }
    int arr[sizeOfHuffArray-1];
    findCodeForLengthOrCharacter(root->nodeForCharacterOrLength, arr, 0);

}
void startHuffmanCoddingForFlagAndOffset(uint16_t * flagAndOffset, int * frequencyArray,int sizeOfHuffArray)
{
    int len = sizeOfHuffArray;

    int i,sumFrequeny;
    struct huffmanTreeForFlagAndOffset *temp ;
    initHuffmanTreeForFlagAndOffset(flagAndOffset,frequencyArray,len);// karakterler ve frekanslari huffman agacina ekleniyor.

    for(i=0; i<len-1; i++)
    {
        sumFrequeny = rootFlagAndOffset->nodeForFlagAndOffset->frequency + rootFlagAndOffset->next->nodeForFlagAndOffset->frequency; // toplam frekanslar
        // karakterler disinde toplamlar oldugunda default olarak NULL degeri atanir.
        addNodeToHuffmanTreeForFlagAndOffset(0,sumFrequeny,rootFlagAndOffset->next->nodeForFlagAndOffset,rootFlagAndOffset->nodeForFlagAndOffset);
        temp = rootFlagAndOffset->next->next;
        rootFlagAndOffset=temp;
    }
    int arr[sizeOfHuffArray];
    findCodeForFlagAndOffset(rootFlagAndOffset->nodeForFlagAndOffset, arr, 0,sizeOfHuffArray);// huffman agacindaki kodlamalar cekilecek.

}


void printArr(int a )
{
    if(a)
    {
        for (int i =0; i<sizeOfElementsArray-1; i++)
        {
            printf("karakter:%c code:%d \n",huffmanElementsArray[i],huffmanCodingArray[i] );
        }
    }
    else
    {
        for(int i =0 ; i<sizeOfElementsArray-1; i++)

        {
            printf("karakter:%d code: ",flagAndOffsetHuffArray[i] );
            for(int j =0; huffmanCodingArrayForFlagAndOffset[i].array[j] != -1; j++ )
            {
                printf("%d",huffmanCodingArrayForFlagAndOffset[i].array[j]);
            }
            printf("\n");
        }

    }


}
void  findFrequencyForLengthOrCharacter(struct lzSSToken *tokenArray, int size,int *sizeOfHuffArray)
{


    characterOrLengthFrequency= malloc(sizeof(int ));
    charactersHuffArray =malloc(sizeof(uint8_t)) ;
    int i;
    int tempArr[256];
    int index=0;
    for(i=0; i<256; i++)
    {
        tempArr[i]=0;// tum karakterlerin tekrar sayisina 0 atadik.
    }
    for(i=0; i<size-1; i++)
    {
        if(tokenArray[i].lengthOrCharacter!= -1)
            tempArr[tokenArray[i].lengthOrCharacter]++ ; // o indistekti karakterin tekrar sayisi bir artiriliyor.

    }
    for(i=0; i<256; i++)
    {
        if(tempArr[i]!=0 )
        {
            /*
            if(index>currentSize)
            {
                currentSize+=100;
                characterOrLengthFrequency= realloc(characterOrLengthFrequency,sizeof(int)*currentSize);
                charactersHuffArray = realloc(charactersHuffArray, sizeof(uint8_t)*currentSize);
            }*/

            characterOrLengthFrequency= realloc(characterOrLengthFrequency,sizeof(int)*(index+1));
            charactersHuffArray = realloc(charactersHuffArray, sizeof(uint8_t)*(index+1));
            characterOrLengthFrequency[index] = tempArr[i];
            //printf("f:%d ,%d\n",characterOrLengthFrequency[index],tempArr[i]);
            charactersHuffArray[index] = i; // karakter karakter arrayina ekleniyor.
            index++;
        }
    }
    *sizeOfHuffArray = index;
    /*
     for(i=0; i<index; i++)
     {
         printf("char :%d  fre : %d\n",charactersHuffArray[i],frequncyArray[i]);
     }*/

}
void  findFrequencyForFlagAndOffset(struct lzSSToken *tokenArray, int size,int *sizeOfHuffArray)
{


    flagAndOffsetFrequency= malloc(sizeof(int ));
    flagAndOffsetHuffArray =malloc(sizeof(uint16_t)) ;
    int i;
    int tempArr[65535];
    int index=0;
    for(i=0; i<65535; i++)
    {
        tempArr[i]=0;// tum karakterlerin tekrar sayisina 0 atadik.
    }
    for(i=0; i<size-1; i++)
    {

        tempArr[tokenArray[i].flagAndOffset]++ ; // o indistekti karakterin tekrar sayisi bir artiriliyor.

    }
    for(i=0; i<65535; i++)
    {
        if(tempArr[i]!=0 )
        {
            /*
            if(index>currentSize)
            {
                currentSize+=100;
                flagAndOffsetFrequency= realloc(flagAndOffsetFrequency,sizeof(int)*currentSize);
                flagAndOffsetHuffArray = realloc(flagAndOffsetHuffArray, sizeof(uint8_t)*currentSize);
            }*/

            flagAndOffsetFrequency= realloc(flagAndOffsetFrequency,sizeof(int)*(index+1));
            flagAndOffsetHuffArray = realloc(flagAndOffsetHuffArray, sizeof(uint16_t)*(index+1));
            flagAndOffsetFrequency[index] = tempArr[i];
            //printf("f:%d ,%d\n",flagAndOffsetFrequency[index],tempArr[i]);
            flagAndOffsetHuffArray[index] = i; // karakter karakter arrayina ekleniyor.
            //printf("%d\n",i);
            index++;
        }
    }
    *sizeOfHuffArray = index;
    /*
     for(i=0; i<index; i++)
     {
         printf("char :%d  fre : %d\n",flagAndOffsetHuffArray[i],frequncyArray[i]);
     }*/

}
void huffmanWriteToFile(struct lzSSToken *encodeArray,int sizeOfTokenArray,char *fileName)
{
    // Bu fonksiyon tokenleri huffman algoritmasinaa sokucak ve
    //dosyaya yazacak.

    FILE *fptr = fopen(fileName,"a");
    if(!fptr)
    {
        printf("deflateOutput dosyasina yazilamadi\n");
        return;
    }
    int *sizeOfHuffArray;
    int *sizeOfHuffArrayForOf,i,j;

    findFrequencyForLengthOrCharacter(encodeArray,(int)sizeOfTokenArray,&sizeOfHuffArray); // frekanslari buluyoruz.
    startHuffmanCoddingForCharOrLength(charactersHuffArray,characterOrLengthFrequency,(int)sizeOfHuffArray);
    findFrequencyForFlagAndOffset(encodeArray,(int)sizeOfTokenArray,&sizeOfHuffArrayForOf);
    startHuffmanCoddingForFlagAndOffset(flagAndOffsetHuffArray,flagAndOffsetFrequency,(int)sizeOfHuffArrayForOf);

    PrintInorderCharacterOrLength(fptr,binaryTreeCOLRoot,fileName);// huffman agaclari yaziliyor
    PrintInorderOffsetAndFlag(fptr,binaryTreeFAndORoot,fileName);

    int buf =0,nbuf=0;
    int *codeArray ;


    for(i =0; i<sizeOfTokenArray-1; i++)
    {
        codeArray = (int *)FindOffsetAndFlag(binaryTreeFAndORoot,encodeArray[i].flagAndOffset);

        for (j = 0; codeArray[j]!=-1; j++)
        {

            buf |= codeArray[j] << nbuf;
            nbuf++;
            if (nbuf == 8)
            {
                fputc(buf, fptr);
                nbuf = buf = 0;
            }
        }
        fputc(buf, fptr);

    }
    buf = nbuf = 0;
    for( i =0 ; i<sizeOfTokenArray-1; i++)
    {
        codeArray = (int *)FindCharacterOrLength(binaryTreeCOLRoot,encodeArray[i].lengthOrCharacter);

        for (j = 0; codeArray[j]!=-1; j++)
        {

            buf |= codeArray[j] << nbuf;
            nbuf++;
            if (nbuf == 8)
            {
                fputc(buf, fptr);
                nbuf = buf = 0;
            }
        }
        fputc(buf, fptr);

    }
    binaryTreeCOLRoot = NULL;
    binaryTreeFAndORoot = NULL;
    fclose(fptr);
}

// bu fonksiyon compress islemi yapacak.
void deflateEncode (char *text,int textLenght,char *fileName)
{
    // deflacte compress algoritmasi olacak.
    struct huffmanTreeForCharacterOrLength * root = NULL; // agac baslangic olarak bos.
    int sizeOfTokenArray =1,blokSize=0;
    // blok size ile boyutlara bolecegiz
    FILE *fptr =  fopen(fileName,"w"); // dosyanin icerigini silecegiz.
    if(!fptr)
    {
        printf("%s dosyasi acilamadi",fileName);
    }
    char *lookAHeadBuffer,*searchBuffer;// arama ve ileri tampon pointerlari olusturuldu.
    int maxLength=0,len;// bu degiskenler dolasim aninda kullanilacak.
    int offset,flag;
    uint8_t lengthOrCharacter=0;
    struct lzSSToken *encodeArray = malloc(sizeof(struct lzSSToken)); // lzSSToken arrayi icin bir yer aciyoruz.
    char *maxtMatchLocation ;//maxsimum eslesmenin basladigi yeri tutan lzSSToken.

    // metinin basindan baslayarak sonuna gidiyoruz. bunu lookahead buffer ile yapacagiz.
    //bu dongu window u degistirecek.
    for(lookAHeadBuffer=text; lookAHeadBuffer < text + textLenght ; lookAHeadBuffer++)
    {
        struct lzSSToken t  ; //lzSSToken arrayine eklencek lzSSToken
        if(lookAHeadBuffer!=text)
            encodeArray = realloc(encodeArray,sizeof(struct lzSSToken)*(sizeOfTokenArray));
        // tokenler icin yer actik.
        searchBuffer = lookAHeadBuffer-SS_SEARCHBUFFER; //arama tamponunu ileri tampondan arama tamponunun uzunlugu kadar gerisine koyduk.
        if(searchBuffer< text )
            searchBuffer = text; // arama tamponunun textin disina cikmasini engelliyoruz.
        //eger eslesme saglanmayacaksa varsayilan degerler atiliyor.


        // buffer search bufferin uzerinde gezinecek.
        maxLength = 0;// lzss icin eger 3 ve 3 ten fazla eslesmeler araniyor.
        maxtMatchLocation=searchBuffer;
        for(; searchBuffer<lookAHeadBuffer; searchBuffer++) // eslesen karakeri search buffer icinde ariyoruz..
        {


            len =findLengthOfMatch(searchBuffer,lookAHeadBuffer,SS_LOOKAHEADBUFFER);// eslesmenin uzunlugunu buluyoruz.
            if(len>2) // maxsimum eslemeye gore
            {
                maxLength=len ;
                maxtMatchLocation=searchBuffer ; // eger esleme maxsimum uzunluga gore baslangic yerini kaydediyoruz.
            }


        }

        if(lookAHeadBuffer+maxLength >=text+textLenght)
            maxLength = text + textLenght - lookAHeadBuffer - 1; // eger son karakter de tekrara dahilse text disina cikmamasin icin azalttik.


        if(maxLength>2)
        {
            offset=lookAHeadBuffer-maxtMatchLocation; // eger eslesme uzunlugu sifirdan buyukse eslesen karakterden uzakligi hesaplandi.
            flag = 1; // bayrak 1 yapilarak eslesmenin oldugu belirtiliyor.
            lengthOrCharacter=maxLength;

        }
        else  // eslesme yokken.
        {
            offset =0 ;
            flag = 0;
            lengthOrCharacter= *(lookAHeadBuffer); // bulundugumuz karakter ekleniyor.

        }
        // struct a elemanlar ataniyor.
        t.lengthOrCharacter = lengthOrCharacter;
        t.flagAndOffset = SETFLAGANDOFFSET(flag,offset);

        encodeArray[sizeOfTokenArray-1]= t; // lzSSToken arrayina pencereden buldugumuz tokene atama yapiyouz.
        sizeOfTokenArray++;
        lookAHeadBuffer += maxLength; // ileri tamponu eslesen uzunluk kadar oteliyoruz.
        blokSize+=maxLength;

        if(blokSize>65000)  // blogun boyutu 65 bin bayti geciyorsa blok sonlandirilacak.
        {
            float yuzde = (((lookAHeadBuffer-text)*100)/(float)textLenght);
            clrscr();// ekran temizleniyor.
            printf("Lz77 ve deflate algoritmalari metin.txt dosyasinin boyutunu kucultuyor.\n");
            printf("Lz77 algoritmasi islemi tamamladi\n");
            printf("Deflate :%c%f\n",'%',yuzde);
            huffmanWriteToFile(encodeArray,sizeOfTokenArray,fileName);
            binaryTreeCOLRoot = NULL;
            binaryTreeFAndORoot=NULL;
            free(flagAndOffsetFrequency);
            free(flagAndOffsetHuffArray);
            free(characterOrLengthFrequency);
            free(charactersHuffArray);
            free(encodeArray);
            sizeOfTokenArray =1;
            encodeArray = malloc(sizeof(struct lzSSToken));
            blokSize=0;
        }
    }
    if(sizeOfTokenArray>1)
        huffmanWriteToFile(encodeArray,sizeOfTokenArray,fileName);
}



void LZSSwriteToFile(struct lzSSToken *array,char * fileName,int sizeOfTokenArray)
{
    FILE *f =fopen(fileName,"wb") ;
    if(f)  // binary yazma kipinde aciliyor.
    {
        fwrite(array,sizeof(struct lzSSToken),sizeOfTokenArray,f);
        fclose(f);
    }
    else
        printf("%s dosyasina yazilimadi.",fileName);

}
int learnSizeOfFile(char * fileName)
{
// programin sonunda olusturulan cikti ornegini inceliyecegiz .
    FILE *f =fopen(fileName,"rb");
    char * text =0; // dondurelecek text
    if (f )
    {
        fseek(f,0,SEEK_END);
        return ftell(f);

        fclose(f); // dosya kapatiliyor.
    }
    else
    {
        printf("%s dosyasi okunurken hata olustu ve dosyanin boyutu hesaplanamadi.",fileName);
        return -1;
    }

}


int main()
{
    // huffman codlamasi icin global degiskenlere hafizadan yer ayridi.
    huffmanElementsArray =(char*) malloc(sizeof(char)) ; // bu degisken hufaman agacindaki elemanlari tutacak.
    huffmanCodingArray = malloc(sizeof(int)) ;
    huffmanCodingArrayForFlagAndOffset = malloc(sizeof(struct huffmanCodeArrayStruct));
    huffmanElementsArrayForFlagAndOffset = malloc(sizeof(int));

    float *textSize;
    char * deflateOutputFileName = "deflateCikti.txt";
    char * lz77OutputFileName = "lz77Cikti.txt";
    char * sourceFileName ="metin.txt";
    char *text = readSourceFile(sourceFileName,&textSize);


    printf("Lz77 ve deflate algoritmalari metin.txt dosyasinin boyutunu kucultuyor.\n");
    LZ77encode(text,(int)textSize,lz77OutputFileName);
    printf("Lz77 algoritmasi islemi tamamladi\n");
    deflateEncode(text,(int)textSize,deflateOutputFileName);
    printf("deflate algoritmasi islemi tamamladi\n");

    // cikti dosyalarinin boyutu ogreniliyor.
    int lz77FileSize = learnSizeOfFile(lz77OutputFileName);
    int deflateFileSize = learnSizeOfFile(deflateOutputFileName);
    float inputFileSize = learnSizeOfFile(sourceFileName);
    printf("Dosya boyutlari :\n");
    printf("metin.txt:%d bayt\n",textSize);
    printf("lz77Cikti.txt:%d bayt\n",lz77FileSize);
    printf("deflateCikti.txt:%d bayt\n",deflateFileSize);


    float lz77CompressPerCent = ((inputFileSize-lz77FileSize)*100) / inputFileSize ;
    float deflateCompressPerCent = ((inputFileSize-deflateFileSize)*100) / inputFileSize;

    printf("Lz77 compress orani (yuzdelik): %f\n", lz77CompressPerCent);
    printf("deflate compress orani (yuzdelik): %f\n", deflateCompressPerCent);

    printf("Sonuc:\n");
    if(lz77FileSize<deflateFileSize)
        printf("\tLz77 algoritmasi daha iyi kucultme islemi yapmistir.");

    else if (lz77FileSize>deflateFileSize)
        printf("\tdeflate algoritmasi daha iyi kucultme islemi yapmistir.");
    else
        printf("iki algoritma ayni oranda kucultme yapmistir.");
    getchar(); // console ekranı bekletiliyor.
    return 0;
}
