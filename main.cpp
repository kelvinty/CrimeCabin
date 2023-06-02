#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <windows.h>
#include <locale.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#define QtdCenas  42

//STRUCTS

typedef struct botao{
	char *nome;
	int x;
	int y;
	int largura;
	int altura;
	bool clicado;
	void* imagem;
	void* mascara;
} Botao;

typedef struct vetor_botoes{
	int capacidade;
	int tamanho;
	Botao *botoes;
} BotoesVetor; 


typedef struct item{
	int id;
	char *nome;
	void *imagem;
	void *mascara;
	void *mini_imagem;
	void *mini_mascara;
	int x;
	int y;
	int largura;
	int altura;
} Item;

typedef struct vetor_itens{
	int capacidade;
	int tamanho;
	Item *itens;
} ItensVetor; 

typedef struct final {
	int id;
	ItensVetor *itens;
	char *descricao;
	char *historia; 
} Final;

typedef struct vetor_finais {
	int capacidade;
	int tamanho;
	Final *finais;
} FinaisVetor;

struct TInventario {
	int maxItens;
	int qtdItens;
	ItensVetor *itens;
	int x;
	int y;
};

typedef struct saida {
	int id;
	char* nome;
	int x;
	int y;
	int largura;
	int altura;
	FinaisVetor *finais;
}Saida;

struct TCamera {
	int id;
	int qtdItens;
	void *imagem;
	ItensVetor *itens;
	Saida *saida;
};

//VARIAVEIS GLOBAIS
//int pg = 1;
int last_time = clock();

//Loops do jogo;
int comecaJogo();
int Tutorial();
int Menu();
int Conclusao(Final final);
int animacao_porta();

//Funções do jogo

void* load_image(const char *endereco, int largura, int altura, int x, int y){
	int aux = imagesize(x,y,largura,altura);
	void*img = malloc(aux);
	readimagefile(endereco, x, y, largura, altura);
	getimage(x, y, largura, altura, img);
	return img;
}

int strlen(char *str)
{
    int total=0;
    while( str[total] != '\0'){
    	total++;
	}    
    return total;
}

int animacao_texto(char *texto,int quebra_linha,int qtd,int pos_x,int pos_y,unsigned long long int ts1) {
	int pg = 1;
	unsigned long long int ts2 = GetTickCount();
	
	double larguraString = textwidth(texto);
	double alturaString = textheight(texto);
	
	int qtdLetrasTexto = strlen(texto);
	int qtdLinhas = ceil(larguraString/quebra_linha);	
	int larguraLetras = larguraString/qtdLetrasTexto;
	int qtdLetrasLinha = ceil(quebra_linha/larguraLetras);

	char *str = (char*)malloc(sizeof(char)*qtdLetrasTexto);

  	int letraAtual = 0;
  	
  	waveOutSetVolume(0,0x88888888);
  	mciSendString("seek Teclado to start", NULL, 0, 0);
	mciSendString("play Teclado repeat", NULL, 0, 0);
  	
  	//loop para guardar os textos separados em linhas
  	for(int k = 0;k < qtdLinhas;k++) {
  		//percorre um loop sobre as linhas do texto
  		for(int j = 0;j< qtdLetrasLinha || textwidth(str) < quebra_linha;j++){
  			if (pg == 1) pg = 2; else pg=1;
			setvisualpage(pg);
  			//percorre um loop para cada letra a ser adicionada na linha atual
  			str[j] = texto[letraAtual]; //adiciona a letra do texto no indice atual + a posição da linha ex: linha 1 * qtdLetras = texto[183] = "a";
			str[j+1] = '\0';//adiciona a trava de leitura de string no final da lista 
			outtextxy(pos_x ,pos_y + (k*alturaString),str);//denhando o texto na tela
			delay(65);
			letraAtual++;
			setactivepage(pg);
			if(letraAtual > qtdLetrasTexto) {
				break;//sai do loop quando a atingir a quantidade de letras
			}
		}
		if(letraAtual > qtdLetrasTexto) {
			break;//sai do loop quando a atingir a quantidade de letras
		}
		outtextxy(pos_x, pos_y + (k*alturaString), str);//desenha a linha do texto passado na tela
        setactivepage(pg);
	}
	
	mciSendString("stop Teclado", NULL, 0, 0);
	delay(3000);
	return 0;

}

void deleteImage(void *image){
	free(image);
}

Item *criar_item(int id, char *nome, void *imagem, void *mascara, void*mini, void*mini_mask, int x, int y, int largura, int altura) {
	Item *item = (Item*)malloc(sizeof(Item));
	
	item->id = id;
	item->nome = nome;
	item->imagem = imagem;
	item->mascara = mascara;
	item->mini_imagem = mini;
	item->mini_mascara = mini_mask;
	item->x = x;
	item->y = y;
	item->largura = largura;
	item->altura = altura;
	
	return item;
}

void apaga_item(Item **item_ref) {
	Item *item = *item_ref;
	free(item);
	*item_ref = NULL;
}

ItensVetor *criar_vetor_itens(int capacidade){
	ItensVetor *vec = (ItensVetor*) calloc(1,sizeof(ItensVetor));

	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->itens = (Item*) calloc(capacidade,sizeof(Item));

	return vec;
}

void print_vetor_itens(ItensVetor *vec){
	for(int i = 0;i < vec->tamanho;i++){
		printf("item:%s, indice:%d\n",vec->itens[i].nome,i);
	}
}

void apaga_vetor_itens(ItensVetor **vec_ref){
	ItensVetor *vec = *vec_ref;
	
	free(vec->itens);
	free(vec);
	
	*vec_ref = NULL;
}

void append_vetor_itens(ItensVetor *vec, Item *item){
	if(vec->tamanho == vec->capacidade){
		printf("vetor cheio");
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->itens[vec->tamanho] = *item;

	vec->tamanho++;
}

int compara_vetor_itens(Final final,const ItensVetor *vec_inventario){
	int count = 0;
	
	ItensVetor *vec_final = final.itens;
	
	for(int i = 0;i < vec_final->tamanho;i++){
		Item item_final = vec_final->itens[i];
		for(int j = 0;j < vec_inventario->tamanho; j++){
			Item item_inventario = vec_inventario->itens[j];
			printf("item final id:%d , item final inventario: %d\n",item_final.id,item_inventario.id);
			if(item_final.id == item_inventario.id){
				count++;
			} 
		}
	}
	return count;
}

void remove_item_vetor(ItensVetor *vec,Item *item){
	if(vec->tamanho > 0) {
		for(int i = 0;i < vec->tamanho;i++){
			if(vec->itens[i].id == item->id) {
				for(int j = i; j < vec->tamanho -1;j++) {
//					printf("\nachou id:%d | item_id:%d",vec->itens[i].id,item->id);
					vec->itens[j] = vec->itens[j+1];
				}	
				vec->tamanho--;
			}
		}
	}
}

Botao *criar_botao(char*nome, void *imagem, void *mascara, int x, int y, int largura, int altura) {
	Botao *botao = (Botao*)malloc(sizeof(Botao));
	
	botao->nome = nome;
	botao->imagem = imagem;
	botao->mascara = mascara;
	botao->x = x;
	botao->y = y;
	botao->largura = largura;
	botao->altura = altura;
	
	return botao;
}

BotoesVetor *criar_vetor_botoes(int capacidade){
	BotoesVetor *vec = (BotoesVetor*) calloc(1,sizeof(BotoesVetor));
	
	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->botoes = (Botao*) calloc(capacidade,sizeof(Botao));
	
	return vec;
}

void append_vetor_botoes(BotoesVetor *vec, Botao *botao){
	if(vec->tamanho == vec->capacidade){
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->botoes[vec->tamanho] = *botao;

	vec->tamanho++;
}

void apaga_vetor_botoes(BotoesVetor **vec_ref){
	BotoesVetor *vec = *vec_ref;
	
	free(vec->botoes);
	free(vec);
	
	*vec_ref = NULL;
}


void apaga_botao(Botao **botao_ref) {
	Botao *botao = *botao_ref;
	free(botao);
	*botao_ref = NULL;
}

Final *criar_final(int _id, char *_descricao, char *_historia) {
	Final *final = (Final*)calloc(1,sizeof(Final));
	
	final->id = _id;
	final->itens = NULL;
	final->descricao = _descricao;
	final->historia = _historia;
	
	return final;
}

void apaga_final(Final **final_ref) {
	Final *final = *final_ref;
	free(final);
	*final_ref = NULL;
}

FinaisVetor *criar_vetor_finais(int capacidade){
	FinaisVetor *vec = (FinaisVetor*) calloc(1,sizeof(FinaisVetor));
	
	vec->tamanho = 0;
	vec->capacidade = capacidade;
	vec->finais = (Final*) calloc(capacidade,sizeof(Final));
	
	return vec;
}

void print_vetor_finais(FinaisVetor *vec){
	for(int i = 0;i < vec->tamanho;i++){
		printf("final:%s, indice:%d\n",vec->finais[i].descricao,i);
	}
}

void apaga_vetor_finais(FinaisVetor **vec_ref){
	FinaisVetor *vec = *vec_ref;
	
	free(vec->finais);
	free(vec);
	
	*vec_ref = NULL;
}

void append_vetor_finais(FinaisVetor *vec, Final *final){
	if(vec->tamanho == vec->capacidade){
		fprintf(stderr,"ERROR in 'append'\n");
		fprintf(stderr,"Vetor cheio'\n");
		exit(EXIT_FAILURE);
	}
	
	vec->finais[vec->tamanho] = *final;

	vec->tamanho++;
}

void remove_final_vetor(FinaisVetor *vec,Final *final){
	if(vec->tamanho > 0) {
		for(int i = 0;i < vec->tamanho;i++){
			if(vec->finais[i].id == final->id) {
				for(int j = i; j < vec->tamanho -1;j++) {
//					printf("\nachou id:%d | item_id:%d",vec->itens[i].id,item->id);
					vec->finais[j] = vec->finais[j+1];
				}	
				vec->tamanho--;
			}
		}
//		print_vetor_finais(vec);
	}
}

TInventario *criar_inventario(int x, int y) {
	TInventario *inventario = (TInventario*)calloc(1,sizeof(TInventario));
	
//	inventario->maxItens = maxItens;
//	inventario->qtdItens = qtdItens;
	inventario->x = x;
	inventario->y = y;
	inventario->itens = NULL;
	
	return inventario;
}



Saida *criarSaida(int _id,char*_nome,int _x, int _y,int largura, int altura) {
	Saida *saida = (Saida*)calloc(1,sizeof(Saida));
	
	saida->id = _id;
	saida->nome = _nome;
	saida->x = _x;
	saida->y = _y;
	saida->largura = largura;
	saida->altura = altura;
	saida->finais = NULL;
	
	return saida;
}


//CRUD CAMERA

TCamera *criarCamera(int _id,void*_imagem) {
	TCamera *camera = (TCamera*) calloc(1,sizeof(TCamera));
	
	camera->imagem = _imagem;
	camera->id = _id;
	camera->itens = NULL;
	camera->saida = NULL;
	
	return camera;
}

void mostrarCamera(const TCamera *camera) {
	putimage(0,0,camera->imagem,COPY_PUT);
}

void mostrarItensCamera(const TCamera *camera) {
	for(int i =0;i < camera->itens->tamanho;i++) {
		Item item = camera->itens->itens[i]; 
		putimage(0, 0, item.mascara, AND_PUT);
		putimage(0, 0, item.imagem, OR_PUT);
	}
}

void mostrarSaidasCamera(const TCamera *camera) {
	Saida *saida = camera->saida; 
	if(saida != NULL){
		rectangle(saida->x,saida->y,saida->largura + saida->x,saida->altura+ saida->y);
	}

}

void mostrarInventario(const TInventario *inventario,void *img) {
	putimage(inventario->x,inventario->y,img,COPY_PUT);
}

void mostrarItensInventario(const TInventario *inventario) {
	for(int i =0;i < inventario->itens->tamanho;i++) {
		Item item = inventario->itens->itens[i]; 
		putimage(inventario->x, inventario->y + item.altura*i, item.mini_mascara, AND_PUT);
		putimage(inventario->x, inventario->y + item.altura*i, item.mini_imagem, OR_PUT);
	}
}

void mostrarBotoes(const BotoesVetor *botoes) {
	for(int i = 0;i< botoes->tamanho;i++){
		Botao botao = botoes->botoes[i];
		putimage(botao.x, botao.y, botao.mascara, AND_PUT);
		putimage(botao.x, botao.y, botao.imagem, OR_PUT);
	}
	
}

void mostraTempo(int tempo, void*hud_tempo, void*hud_tempo_m){

	bool chamou1;
	bool chamou2;
	bool chamou3;
	bool chamou4;
	
	int last_time = GetTickCount();
	char str[20];
	int tempo_limite = 15;
	int segundos = (last_time - tempo)/1000;
	float milisegundos = (last_time - tempo);
	int count = 0;
	int gt2 = GetTickCount();
	
	if(segundos > 15) {
		segundos = 15;
		mciSendString("stop clock", NULL, 0, 0);
	}
	
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,4);
	sprintf(str,"%d",tempo_limite - segundos);
	int tamanho_texto = textwidth(str);
	
	putimage(1280/2 - 150/2 ,0,hud_tempo_m,AND_PUT);
	putimage(1280/2 - 150/2 ,0,hud_tempo,OR_PUT);
	
	
	outtextxy(1280/2 - tamanho_texto/2 ,22,str);

	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
}

void executaSom(unsigned long long int *ts1){
	unsigned long long int ts2 = GetTickCount();
	waveOutSetVolume(0,0xFFFFFFFF);
	int randomic_sound = rand() % 3;
	if((ts2 - *ts1) >= 5000 ){
		if(randomic_sound == 0){
			mciSendString("seek Porta1 to start", NULL, 0, 0);
			mciSendString("play Porta1", NULL, 0, 0);
			*ts1 = GetTickCount();
		} else if (randomic_sound == 1){
			mciSendString("seek Porta2 to start", NULL, 0, 0);
			mciSendString("play Porta2", NULL, 0, 0);
			*ts1 = GetTickCount();
		} else if (randomic_sound == 2){
			mciSendString("seek Porta3 to start", NULL, 0, 0);
			mciSendString("play Porta3", NULL, 0, 0);
			*ts1 = GetTickCount();
		}
		
	}
}

void mostrarBotao(const Botao *botao) {
	putimage(botao->x, botao->y, botao->mascara, AND_PUT);
	putimage(botao->x, botao->y, botao->imagem, OR_PUT);
}

void removeItensCamera(TCamera *camera) {
	for(int i =0;i<camera->qtdItens;i++) {
		Item *item = camera->itens->itens; 
		free(item->imagem);
		free(item->mascara);
		free(item);
	}
}

bool clicou = false;
//

//
bool verificaMouseClick() {
	int x;
	int y;
	bool clicou = false;
	if(ismouseclick(WM_LBUTTONDOWN)){
		getmouseclick(WM_LBUTTONDOWN,x,y);
		clearmouseclick(WM_LBUTTONDOWN);
		clicou = true;
	}
	return clicou;
}


void pegarItem(Item *_item, TCamera *camera, TInventario *inventario){
	unsigned long long int ts1 = GetTickCount();
	unsigned long long int ts2 = GetTickCount();
	int pg = 1;
	for(int i = 0;i<camera->itens->tamanho;i++){
		Item item = camera->itens->itens[i];
		if(item.id == _item->id){
			if(inventario->itens->tamanho >= inventario->itens->capacidade){
				while(ts2-ts1 < 1000){
					ts2 = GetTickCount();
					if(pg == 1) pg = 2; else pg = 1;
					setvisualpage(pg);
//					cleardevice();
					settextstyle(SANS_SERIF_FONT,HORIZ_DIR,4);
					outtextxy(1280/2 - textwidth("inventario cheio!")/2 ,720 -100,"inventario cheio!");
					outtextxy(1280/2 - textwidth("Escolha uma saída!")/2 ,720 -70,"Escolha uma saída!");
					setactivepage(pg);
				}
			} else {
				waveOutSetVolume(0,0xFFFFFFFF);
				mciSendString("seek Item to start", NULL, 0, 0);
				mciSendString("play Item", NULL, 0, 0);
				Item item = camera->itens->itens[i];
				remove_item_vetor(camera->itens,&item);
				append_vetor_itens(inventario->itens,&item);
			}
			
		}
	}

}

void colisaoMouseBotao(BotoesVetor* botoes){
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();
  	
  	if (GetCursorPos(&P)) // captura a posição do mouse. A variável P é carregada com coordenadas físicas de tela
        if (ScreenToClient(janela, &P)) 
  	
	for(int i = 0;i<botoes->tamanho;i++) {
		Botao botao = botoes->botoes[i];

		if (P.x < botao.x + botao.largura && P.x > botao.x && P.y < botao.y + botao.altura && P.y > botao.y) {

			bar(botao.x,botao.y + (botao.altura - 2.5),botao.x+botao.largura,botao.y+botao.altura);
			delay(50);
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			if(botao.nome == "iniciar"){
    				mciSendString("stop Tema", NULL, 0, 0);
    				comecaJogo();
				} else if(botao.nome == "intro") {
					delay(200);
					Tutorial();
				} else if(botao.nome == "voltar") {
					delay(200);
					Menu();
				}  else if (botao.nome == "sair") {
					exit(EXIT_FAILURE);
				} else if (botao.nome == "sim"){
					comecaJogo();
				} else if (botao.nome == "nao"){
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}

void colisaoMouseItens(TCamera camera, TInventario *inventario) {
	
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();
  	
  	if (GetCursorPos(&P)) // captura a posição do mouse. A variável P é carregada com coordenadas físicas de tela
        if (ScreenToClient(janela, &P)) 
  	
	for(int i = 0;i<camera.itens->tamanho;i++) {
		Item item = camera.itens->itens[i];
		rectangle(item.x,item.y,item.largura+item.x,item.altura+item.y);
	
		
		if (P.x < item.x + item.largura && P.x > item.x && P.y < item.y + item.altura && P.y > item.y) {
			printf("x:%d, y:%d, item:%s \n", item.x,item.y,item.nome);
			int largura_texto = textwidth(item.nome);
			outtextxy(item.x + ((item.largura/2) - (largura_texto/2)), item.y+item.altura,item.nome);
			delay(50);
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			pegarItem(&item,&camera,inventario);
			}

		}
	}
	
}

void colisaoMouseSaidas(TCamera camera, TInventario *inventario,Final generico) {
	POINT P;
  	HWND janela;
  	janela = GetForegroundWindow();
  	
  	if (GetCursorPos(&P))
        if (ScreenToClient(janela, &P)) 
  	
	if(camera.saida != NULL) {
		Saida *saida = camera.saida;

		if (P.x < saida->x + saida->largura && 
			P.x > saida->x && 
			P.y < saida->y + saida->altura && 
			P.y > saida->y) 
		{
			int largura_texto = textwidth(saida->nome);
			int altura_texto = textheight(saida->nome);
			outtextxy(saida->x + ((saida->largura/2) - (largura_texto/2)), saida->y + ((saida->altura/2) - (altura_texto/2)),saida->nome);
			
			delay(50);
			
    		if(GetKeyState(VK_LBUTTON)&0x80){
    			int contador = 0;
				printf("clicou na saida : %s\n", saida->nome);			
    			for(int i = 0;i< saida->finais->tamanho; i++){
    				Final final = saida->finais->finais[i];
    				contador = compara_vetor_itens(final,inventario->itens);
    				if(contador == 2){
						printf("foi para o final:%s\n", final.descricao);
						Conclusao(final);
					} 
					printf("terminou de passar os final %d \n",i);
				}
				Conclusao(generico);
				return;
			}	
		}	
	}	
}

void mudarDeCamera(int *camera_atual,char *tecla,unsigned long long int tempo,unsigned long long int *tc2, bool *mudou) {
	
	int VK_A = 0X41;
	int VK_D = 0X44;
		
//	printf("%d\n",tc2);
	
	if(*(mudou) != true){
		if(GetKeyState(VK_A)&0X80) {
			*(tc2) = GetTickCount();
			*camera_atual -= 1;
			if(*camera_atual < 0) {
        		*camera_atual = 3;
			}
			waveOutSetVolume(0,0xFFFFFFFF);
			mciSendString("seek Passos to start", NULL, 0, 0);
			mciSendString("play Passos", NULL, 0, 0);
//			
//			printf("%d\n",tc2);
//			printf("%d\n", tempo);
			*(mudou) = true;
		}
	
		if(GetKeyState(VK_D)&0X80) {
			*(tc2) = GetTickCount();
			*camera_atual += 1;
			if(*camera_atual > 3) {
        		*camera_atual = 0;
			}
			waveOutSetVolume(0,0xFFFFFFFF);
			mciSendString("seek Passos to start", NULL, 0, 0);
			mciSendString("play Passos", NULL, 0, 0);
			*(mudou) = true;
		} 
	}
	if(tempo - *(tc2) > 200){
//		printf("agora pode mudar\n");
		*(mudou) = false;
	}
	
	
}

unsigned long long int gt1 = GetTickCount();

int animacao_splash()  { 
  	int pg, Logo;
  	void **t = (void **) malloc(sizeof(void *) *80); 
	                                       
  	Logo = imagesize(0, 0, 1280,720);	

  	for(int i = 0; i < 80; i++) {
    	t[i] = malloc(Logo);
	}
	
	readimagefile(".\\Splash2\\lunar_00001.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[0]);
	readimagefile(".\\Splash2\\lunar_00002.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[1]);
    readimagefile(".\\Splash2\\lunar_00003.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[2]);
    readimagefile(".\\Splash2\\lunar_00004.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[3]);
    readimagefile(".\\Splash2\\lunar_00005.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[4]);
    readimagefile(".\\Splash2\\lunar_00006.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[5]);
    readimagefile(".\\Splash2\\lunar_00007.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[6]);
    readimagefile(".\\Splash2\\lunar_00008.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[7]);
    readimagefile(".\\Splash2\\lunar_00009.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[8]);
    readimagefile(".\\Splash2\\lunar_00010.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[9]);
    readimagefile(".\\Splash2\\lunar_00011.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[10]);
    readimagefile(".\\Splash2\\lunar_00012.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[11]);
    readimagefile(".\\Splash2\\lunar_00013.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[12]);
    readimagefile(".\\Splash2\\lunar_00014.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[13]);
    readimagefile(".\\Splash2\\lunar_00015.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[14]);
    readimagefile(".\\Splash2\\lunar_00016.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[15]);
    readimagefile(".\\Splash2\\lunar_00017.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[16]);
    readimagefile(".\\Splash2\\lunar_00018.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[17]);
    readimagefile(".\\Splash2\\lunar_00019.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[18]);
    readimagefile(".\\Splash2\\lunar_00020.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[19]);
    readimagefile(".\\Splash2\\lunar_00021.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[20]);
    readimagefile(".\\Splash2\\lunar_00022.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[21]);
    readimagefile(".\\Splash2\\lunar_00023.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[22]);
    readimagefile(".\\Splash2\\lunar_00024.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[23]);
    readimagefile(".\\Splash2\\lunar_00025.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[24]);
    readimagefile(".\\Splash2\\lunar_00026.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[25]);
    readimagefile(".\\Splash2\\lunar_00027.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[26]);
    readimagefile(".\\Splash2\\lunar_00028.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[27]);
    readimagefile(".\\Splash2\\lunar_00029.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[28]);
    readimagefile(".\\Splash2\\lunar_00030.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[29]);
    readimagefile(".\\Splash2\\lunar_00031.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[30]);
    readimagefile(".\\Splash2\\lunar_00032.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[31]);
    readimagefile(".\\Splash2\\lunar_00033.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[32]);
    readimagefile(".\\Splash2\\lunar_00034.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[33]);
    readimagefile(".\\Splash2\\lunar_00035.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[34]);
    readimagefile(".\\Splash2\\lunar_00036.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[35]);
    readimagefile(".\\Splash2\\lunar_00037.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[36]);
    readimagefile(".\\Splash2\\lunar_00038.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[37]);
    readimagefile(".\\Splash2\\lunar_00039.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[38]);
    readimagefile(".\\Splash2\\lunar_00040.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[39]);
    readimagefile(".\\Splash2\\lunar_00041.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[40]);
    readimagefile(".\\Splash2\\lunar_00042.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[41]);
    readimagefile(".\\Splash2\\lunar_00043.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[42]);
    readimagefile(".\\Splash2\\lunar_00044.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[43]);
    readimagefile(".\\Splash2\\lunar_00045.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[44]);
    readimagefile(".\\Splash2\\lunar_00046.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[45]);
    readimagefile(".\\Splash2\\lunar_00047.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[46]);
    readimagefile(".\\Splash2\\lunar_00048.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[47]);
    readimagefile(".\\Splash2\\lunar_00049.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[48]);
    readimagefile(".\\Splash2\\lunar_00050.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[49]);
    readimagefile(".\\Splash2\\lunar_00051.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[50]);
    readimagefile(".\\Splash2\\lunar_00052.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[51]);
    readimagefile(".\\Splash2\\lunar_00053.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[52]);
    readimagefile(".\\Splash2\\lunar_00054.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[53]);
    readimagefile(".\\Splash2\\lunar_00055.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[54]);
    readimagefile(".\\Splash2\\lunar_00056.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[55]);
    readimagefile(".\\Splash2\\lunar_00057.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[56]);
    readimagefile(".\\Splash2\\lunar_00058.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[57]);
    readimagefile(".\\Splash2\\lunar_00059.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[58]);
    readimagefile(".\\Splash2\\lunar_00060.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[59]);
    readimagefile(".\\Splash2\\lunar_00061.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[60]);
    readimagefile(".\\Splash2\\lunar_00062.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[61]);
    readimagefile(".\\Splash2\\lunar_00063.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[62]);
    readimagefile(".\\Splash2\\lunar_00064.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[63]);
    readimagefile(".\\Splash2\\lunar_00065.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[64]);
    readimagefile(".\\Splash2\\lunar_00066.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[65]);
    readimagefile(".\\Splash2\\lunar_00067.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[66]);
    readimagefile(".\\Splash2\\lunar_00066.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[67]);
    readimagefile(".\\Splash2\\lunar_00069.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[68]);
    readimagefile(".\\Splash2\\lunar_00070.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[69]);
    readimagefile(".\\Splash2\\lunar_00071.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[70]);
    readimagefile(".\\Splash2\\lunar_00072.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[71]);
    readimagefile(".\\Splash2\\lunar_00073.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[72]);
    readimagefile(".\\Splash2\\lunar_00074.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[73]);
    readimagefile(".\\Splash2\\lunar_00075.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[74]);
	
    for(double j = 0; j < 74;) {
    	if (pg == 1) pg = 2; else pg=1;
		setvisualpage(pg);
		cleardevice();
		int indi = ceil(j);
      	putimage(0, 0, t[indi], COPY_PUT);
      	j+=0.12;
      	if(indi == 74/2){
      		mciSendString("seek Corvo to start", NULL, 0, 0);
   			mciSendString("play Corvo", NULL, 0, 0);
		}
      	setactivepage(pg);
    }
    
    mciSendString("stop Corvo", NULL, 0, 0);
  	return 0; 
}


int animacao_porta()  { 
  	int pg, Porta, i, um = 1, xt = 0;
  	void **t;                                         
  	t = (void **)malloc(sizeof(void *) * QtdCenas);  
  
  	Porta = imagesize(0, 0, 1280,720);	

  	for(i = 0; i < QtdCenas; i++) {
    	t[i] = malloc(Porta);
	}
	
    readimagefile(".\\Animacao\\K_0001.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[0]);
    readimagefile(".\\Animacao\\K_0002.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[1]);
    readimagefile(".\\Animacao\\K_0003.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[2]);
    readimagefile(".\\Animacao\\K_0004.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[3]);
    readimagefile(".\\Animacao\\K_0005.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[4]);
    readimagefile(".\\Animacao\\K_0006.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[5]);
    readimagefile(".\\Animacao\\K_0007.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[6]);
    readimagefile(".\\Animacao\\K_0008.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[7]);
    readimagefile(".\\Animacao\\K_0009.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[8]);
    readimagefile(".\\Animacao\\K_0010.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[9]);
    readimagefile(".\\Animacao\\K_0011.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[10]);
    readimagefile(".\\Animacao\\K_0012.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[11]);
    readimagefile(".\\Animacao\\K_0013.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[12]);
    readimagefile(".\\Animacao\\K_0014.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[13]);
    readimagefile(".\\Animacao\\K_0015.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[14]);
    readimagefile(".\\Animacao\\K_0016.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[15]);
    readimagefile(".\\Animacao\\K_0017.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[16]);
    readimagefile(".\\Animacao\\K_0018.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[17]);
    readimagefile(".\\Animacao\\K_0019.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[18]);
    readimagefile(".\\Animacao\\K_0020.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[19]);
    readimagefile(".\\Animacao\\K_0021.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[20]);
    readimagefile(".\\Animacao\\K_0022.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[21]);
    readimagefile(".\\Animacao\\K_0023.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[22]);
    readimagefile(".\\Animacao\\K_0024.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[23]);
    readimagefile(".\\Animacao\\K_0025.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[24]);
    readimagefile(".\\Animacao\\K_0026.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[25]);
    readimagefile(".\\Animacao\\K_0027.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[26]);
    readimagefile(".\\Animacao\\K_0028.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[27]);
    readimagefile(".\\Animacao\\K_0029.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[28]);
    readimagefile(".\\Animacao\\K_0030.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[29]);
    readimagefile(".\\Animacao\\K_0031.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[30]);
    readimagefile(".\\Animacao\\K_0032.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[31]);
    readimagefile(".\\Animacao\\K_0033.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[32]);
    readimagefile(".\\Animacao\\K_0034.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[33]);
    readimagefile(".\\Animacao\\K_0035.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[34]);
    readimagefile(".\\Animacao\\K_0036.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[35]);
    readimagefile(".\\Animacao\\K_0037.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[36]);
    readimagefile(".\\Animacao\\K_0038.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[37]);
    readimagefile(".\\Animacao\\K_0039.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[38]);
    readimagefile(".\\Animacao\\K_0040.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[39]);
    readimagefile(".\\Animacao\\K_0041.BMP",0,0,1280,720);
    getimage(0,0,1280,720, t[40]);

	setbkcolor(RGB(0, 0, 0));
	
   	mciSendString("seek Abrindo to start", NULL, 0, 0);
   	mciSendString("play Abrindo", NULL, 0, 0);
    
    for(double j = 0; j < 40;) 
    {
    	if (pg == 1) pg = 2; else pg=1;
		setvisualpage(pg);
		cleardevice();
		int indi = ceil(j);
      	putimage(0, 0, t[indi], COPY_PUT);
      	j+= 0.1;
      	setactivepage(pg);
    }
    
    mciSendString("stop Abrindo", NULL, 0, 0);
    
  	return 0; 
}


int Conclusao(Final final) {
	int count = 0;
	int pg = 1;
	unsigned long long int ts1 = GetTickCount();
	
	mciSendString("stop Insetos", NULL, 0, 0);
	mciSendString("stop clock", NULL, 0, 0);
	
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
	mciSendString("open .\\Audios\\porta-rangendo.mp3 type MPEGVideo alias Abrindo", NULL, 0, 0);
	mciSendString("open .\\Audios\\Teclado.mp3 type MPEGVideo alias Teclado", NULL, 0, 0);
	
	int LarTela;
	LarTela = 1050;
	unsigned long long int gt2;
	
	char *texto = final.historia;
	
	animacao_porta();
 	animacao_texto(texto,LarTela,583,50,50,ts1);

	void *fim_de_jogo = load_image("FimdeJogo.bmp",1280,720,0,0);
	
	void *botao1_img = load_image(".\\Hud\\Sim.bmp",100,50,0,0);
	void *botao1_mask = load_image(".\\Hud\\SimWB.bmp",100,50,0,0);
	
	void *botao2_img = load_image(".\\Hud\\Nao.bmp",100,50,0,0);
	void *botao2_mask = load_image(".\\Hud\\NaoWB.bmp",100,50,0,0);
	
	Botao *botao_sim = criar_botao("sim",botao1_img,botao1_mask,(1280/2)-180,450,100,50);
	Botao *botao_nao = criar_botao("nao",botao2_img,botao2_mask,(1280/2)+100,450,100,50);
	BotoesVetor *botoes = criar_vetor_botoes(2);
	
	append_vetor_botoes(botoes,botao_sim);
	append_vetor_botoes(botoes,botao_nao);
	
	while(true) {
 		gt2 = GetTickCount();
 		
		if(gt2 - gt1 > 1000/60) {
			
			if(pg == 1) pg = 2; else pg = 1;
 			setvisualpage(pg);
 			
 			cleardevice();
 			
 			putimage(0,0,fim_de_jogo,COPY_PUT);
 			
 			mostrarBotoes(botoes);
			colisaoMouseBotao(botoes);
			
			setactivepage(pg);
		}
	}
	deleteImage(botao1_img);
	deleteImage(botao2_img);
	
	deleteImage(botao1_mask);
	deleteImage(botao2_mask);
	
	deleteImage(fim_de_jogo);
	
	apaga_vetor_botoes(&botoes);
	
	return 0;
}

int comecaJogo(){
	//variaveis do jogo
	unsigned long long int tempo = GetTickCount();
	unsigned long long int gt2;
	unsigned long long int ts1 = GetTickCount();
	unsigned long long int tc2 = GetTickCount();
	
    char tecla = 0;
	int camera_atual = 0;
	int qtdCam = 0;
	int pg = 1;
	int LarTela,AltTela;
	
	bool mudou = false;


	
	LarTela = 1280;
	AltTela = 720;
	
	setlocale(LC_ALL,"Portuguese");
	settextstyle(SANS_SERIF_FONT,HORIZ_DIR,2);
	
	mciSendString("open .\\Audios\\porta1.mp3 type MPEGVideo alias Porta1", NULL, 0, 0);
	mciSendString("open .\\Audios\\porta2.mp3 type MPEGVideo alias Porta2", NULL, 0, 0);
	mciSendString("open .\\Audios\\porta3.mp3 type MPEGVideo alias Porta3", NULL, 0, 0);
	mciSendString("open .\\Audios\\walk.mp3 type MPEGVideo alias Passos", NULL, 0, 0);
	mciSendString("open .\\Audios\\clock.mp3 type MPEGVideo alias clock", NULL, 0, 0);
	mciSendString("open .\\Audios\\BugsSound.mp3 type MPEGVideo alias Insetos", NULL, 0, 0);
	mciSendString("open .\\Audios\\item-equip.mp3 type MPEGVideo alias Item", NULL, 0, 0);
	
	waveOutSetVolume(0,0x88888888);
	mciSendString("seek Insetos to start", NULL, 0, 0);
	mciSendString("play Insetos repeat", NULL, 0, 0);
	
	waveOutSetVolume(0,0x55555555);
	mciSendString("seek clock to start", NULL, 0, 0);
	mciSendString("play clock", NULL, 0, 0);
	
	void* hud_tempo = load_image(".\\Hud\\Relogio.bmp",150,75,0,0);
	void* hud_tempo_mask = load_image(".\\Hud\\RelogioWB.bmp",150,75,0,0);
	void* hud_inventario = load_image(".\\Hud\\Inventario.bmp",100,200,0,0);
	
	
	void* item0_img = load_image(".\\Itens\\camera\\rede.BMP",1280,720,0,0);
	void* item0_mask = load_image(".\\Itens\\camera\\Rede máscara.BMP",1280,720,0,0);
	void* item0_mini = load_image(".\\Itens\\inventario\\rede.BMP",100,100,0,0);
	void* item0_mini_mask = load_image(".\\Itens\\inventario\\Rede ícone máscara maior.BMP",100,100,0,0);
	
	void* item1_img = load_image(".\\Itens\\camera\\gasolina.BMP",1280,720,0,0);
	void* item1_mask = load_image(".\\Itens\\camera\\Gasolina máscara.BMP",1280,720,0,0);
	void* item1_mini = load_image(".\\Itens\\inventario\\gasolina.BMP",100,100,0,0);
	void* item1_mini_mask = load_image(".\\Itens\\inventario\\gasolinaWB.BMP",100,100,0,0);
	
	void* item2_img = load_image("dinamite.bmp",100,100,0,0);
	void* item2_mask = load_image("dinamite_pb.bmp",100,100,0,0);
	void* item2_mini = load_image(".\\Itens\\inventario\\fosforo.BMP",100,100,0,0);
	void* item2_mini_mask = load_image(".\\Itens\\inventario\\fosforoWB.BMP",100,100,0,0);
	
	void* item3_img = load_image(".\\Itens\\camera\\armadilha.BMP",1280,720,0,0);
	void* item3_mask = load_image(".\\Itens\\camera\\Armadilha máscara.BMP",1280,720,0,0);
	void* item3_mini = load_image(".\\Itens\\inventario\\armadilha.BMP",100,100,0,0);
	void* item3_mini_mask = load_image(".\\Itens\\inventario\\armadilhaWB.BMP",100,100,0,0);
	
	void* item4_img = load_image(".\\Itens\\camera\\Shotgun.BMP",1280,720,0,0);
	void* item4_mask = load_image(".\\Itens\\camera\\Shotgun máscara2.BMP",1280,720,0,0);
	void* item4_mini = load_image(".\\Itens\\inventario\\arma.BMP",100,100,0,0);
	void* item4_mini_mask = load_image(".\\Itens\\inventario\\armaWB.BMP",100,100,0,0);
	
	void* item5_img = load_image("dinamite.bmp",100,100,0,0);
	void* item5_mask = load_image("dinamite_pb.bmp",100,100,0,0);
	void* item5_mini = load_image(".\\Itens\\inventario\\rede.BMP",100,100,0,0);
	void* item5_mini_mask = load_image(".\\Itens\\inventario\\redeWB.BMP",100,100,0,0);
	
	void* item6_img = load_image(".\\Itens\\camera\\machado.BMP",1280,720,0,0);
	void* item6_mask = load_image(".\\Itens\\camera\\Machado máscara.BMP",1280,720,0,0);
	void* item6_mini = load_image(".\\Itens\\inventario\\machado.BMP",100,100,0,0);
	void* item6_mini_mask = load_image(".\\Itens\\inventario\\machadoWB.BMP",100,100,0,0);
	
	void* item7_img = load_image(".\\Itens\\camera\\municao.BMP",1280,720,0,0);
	void* item7_mask = load_image(".\\Itens\\camera\\Munição máscara.BMP",1280,720,0,0);
	void* item7_mini = load_image(".\\Itens\\inventario\\municao.BMP",100,100,0,0);
	void* item7_mini_mask = load_image(".\\Itens\\inventario\\municaoWB.BMP",100,100,0,0);
	
	void* item8_img = load_image("dinamite.bmp",100,100,0,0);
	void* item8_mask = load_image("dinamite_pb.bmp",100,100,0,0);
	void* item8_mini = load_image(".\\Itens\\inventario\\dinamite.BMP",100,100,0,0);
	void* item8_mini_mask = load_image(".\\Itens\\inventario\\dinamiteWB.BMP",100,100,0,0);
	
	void* item9_img = load_image("dinamite.bmp",100,100,0,0);
	void* item9_mask = load_image("dinamite_pb.bmp",100,100,0,0);
	void* item9_mini = load_image(".\\Itens\\inventario\\municao.BMP",100,100,0,0);
	void* item9_mini_mask = load_image(".\\Itens\\inventario\\municaoWB.BMP",100,100,0,0);
	
	void* item10_img = load_image("dinamite.bmp",100,100,0,0);
	void* item10_mask = load_image("dinamite_pb.bmp",100,100,0,0);
	void* item10_mini = load_image(".\\Itens\\inventario\\corda.BMP",100,100,0,0);
	void* item10_mini_mask = load_image(".\\Itens\\inventario\\cordaWB.BMP",100,100,0,0);
	
	Item *sinalizador = criar_item(0,"sinalizador",item1_img,item1_mask,item0_mini,item0_mini_mask,200,200,100,100);
    Item *gasolina = criar_item(1,"gasolina",item1_img,item1_mask,item1_mini,item1_mini_mask,300,200,100,100);
    Item *fosforo = criar_item(2,"fosforo",item2_img,item2_mask,item2_mini,item2_mini_mask,400,200,100,100);
    Item *armadilha = criar_item(3,"armadilha",item3_img,item3_mask,item3_mini,item3_mini_mask,500,200,100,100);
    Item *machado = criar_item(4,"machado",item4_img,item4_mask,item4_mini,item4_mini_mask,600,200,100,100);
    Item *rede = criar_item(5,"rede",item5_img,item5_mask,item5_mini,item5_mini_mask,700,200,100,100);
    Item *chave = criar_item(6,"chave",item6_img,item6_mask,item6_mini,item6_mini_mask,900,200,100,100);
    Item *arma = criar_item(7,"arma",item7_img,item7_mask,item7_mini,item7_mini_mask,930,450,300,200);
    Item *dinamite = criar_item(8,"dinamite",item8_img,item8_mask,item8_mini,item8_mini_mask,600,400,100,100);
    Item *municao = criar_item(9,"municao",item9_img,item9_mask,item9_mini,item9_mini_mask,600,400,100,100);
    Item *corda = criar_item(10,"corda",item10_img,item10_mask,item10_mini,item10_mini_mask,500,400,100,100);
    
	Final *final0 = criar_final(0,"Sinalizador + Gasolina","A situação exigia que a única prioridade fosse a minha vida. Se tudo tivesse que ser consumido pelo fogo para que eu escapasse daquele lugar, assim seria. Espalhei o combustível que eu tinha pela porta e de longe esperei pela entrada do sujeito. Em poucos instantes, a porta cedeu. O homem armado só não esperava receber um tiro de sinalizador assim que colocou os pés dentro da minha cabana. O disparo explodiu em uma bola de fogo no momento em que encostou no líquido. Eu assisti tudo pegar fogo, inclusive o sujeito e escapei pela janela. Era o fim da minha velha cabana, mas era o fim de um assassino e de uma noite que nunca será esquecida Você sobrevive.");
	final0->itens = criar_vetor_itens(2);
	append_vetor_itens(final0->itens,sinalizador);
	append_vetor_itens(final0->itens,gasolina);
	
	Final *final1 = criar_final(1,"Fósforo + gasolina","Você sabe que não há muito tempo para pensar e deve desconsiderar sua cabana para salvar sua vida. Espalhando gasolina, você espera a entrada do sujeito. Ao tentar incendiá-lo com um fósforo. Você também é pego pelas chamas, junto da cabana e do assassino. Talvez não tenha sido uma boa ideia incendiar algo tão próximo. Você está morto.");
	final1->itens = criar_vetor_itens(2);
	append_vetor_itens(final1->itens,fosforo);
	append_vetor_itens(final1->itens,gasolina);
	
	Final *final2 = criar_final(2,"Arma + munição","Qual outra saída poderia ter para aquela situação? Quem quer que estivesse lá fora tomaria chumbo antes que pudesse pensar. A porta cede e um sujeito armado é a primeira coisa que avisto. Era um duelo, quem atirasse primeiro teria o privilégio de sair com vida e assim foi feito. Cada um deu o seu disparo. A prontidão foi a minha grande vantagem, o assassino não esperava ser recebido dessa forma. Dois disparos e apenas uma vítima. A última caça da noite estava caída na minha frente.");
	final2->itens = criar_vetor_itens(2);
	append_vetor_itens(final2->itens,arma);
	append_vetor_itens(final2->itens,municao);
	
	Final *final3 = criar_final(3,"Machado + armadilha","Uma armadilha indiscreta, não foi feita para um homem cair. A não ser que usada do jeito certo. Com um machado em mãos, era questão de imobilizar o alvo antes de tentar me aproximar. O sujeito finalmente abriu a porta. Ele estava armado. No mesmo momento, fiz minha fuga pela janela e deixei a surpresa logo abaixo dela. O homem tentou me perseguir. Ele sabia que eu estava indefeso, mas não esperava pelos dentes de metal que haviam ficado no caminho. Quando ouvi os sons, um estalo e um grito, já sabia o que esperar. Não era mais eu que não tinha para onde fugir. Não houve misericórdia");
	final3->itens = criar_vetor_itens(2);
	append_vetor_itens(final3->itens,machado);	
	append_vetor_itens(final3->itens,armadilha);
	
	Final *final4 = criar_final(4,"Rede + chave","Você alcança a chave do carro. Ao entrar, você arremessa uma rede em direção ao assassino e foge rapidamente pela janela. O tempo ganho é suficiente para escapar com o veículo. Alguns disparos atingem o carro, porém sem efeitos. Você sobrevive");
	final4->itens = criar_vetor_itens(2);
	append_vetor_itens(final4->itens,rede);
	append_vetor_itens(final4->itens,chave);
	

	Final *final6 = criar_final(6,"nenhum","Quem está lá fora? Não pode ser nada ruim, certo? Não no meio da floresta. Talvez alguém precise de ajuda. Você abre a porta e é recebido com um golpe. Talvez não tenha sido uma boa ideia...");
	final6->itens = criar_vetor_itens(0);

	
	Final *final8 = criar_final(8,"Sinalizador + Machado","Numa situação dessas, não podia me privar de pedir ajuda. Era uma emergência a qual nunca tive que lidar. Não era um animal, era uma pessoa, outro ser humano. Alcancei meu machado, disparei meu sinalizador pela janela e me preparei para quem quer que fosse. Mas antes que o sinal pudesse chegar para alguém, o sujeito estava lá dentro. Desprevenido, o que me restava era avançar, mas antes que eu pudesse levantar a lâmina, o sujeito apontou uma arma em minha direção. Não houve misericórdia, meu destino já estava decidido.");
	final8->itens = criar_vetor_itens(2);
	append_vetor_itens(final8->itens,sinalizador);
	append_vetor_itens(final8->itens,machado);
	
	Final *final9 = criar_final(9,"Sinalizador + arma","Era a primeira vez que me encontrava em uma situação como essa: ter que caçar um ser humano. Mas era o que o destino pedia. Se fosse assim, que alguém possa me ajudar. Rapidamente disparei o sinalizador para alertar qualquer um que estivesse ao redor. Ao mesmo tempo, a porta cedeu. O homem estava armado e dentro da cabana. À primeira vista que tive, fiz o meu disparo. Não ouve som ou recuo. Cometi um erro que pagaria com a minha vida. Minha arma, que nunca estava vazia, desta vez não tinha um disparo sequer. Era um duelo perdido. Que pelo menos o sinal sirva para encontrarem o meu corpo.");
	final9->itens = criar_vetor_itens(2);
	append_vetor_itens(final9->itens,sinalizador);
	append_vetor_itens(final9->itens,arma);
	
	Final *final10 = criar_final(10,"Sinalizador + munição","Eu precisava de ajuda. Um disparo de sinalizador sempre pode alertar qualquer autoridade ao redor, sempre tenho um comigo. Foi o que fiz. Sabia que teria que aguardar qualquer um que pudesse chegar, então teria de me defender. Peguei um punhado de munições e antes que pudesse pensar em alcançar o meu rifle, ouço a porta ceder. O homem estava lá dentro. Uma ideia desastrosa foi a única alternativa que eu tinha: disparar com o sinalizador. Eu sabia como aquilo poderia terminar, mas frente ao outro único final, era a escolha a ser feita. Escondido, coloquei a bala da espingarda no pequeno revolver. Rapidamente, apontei a arma em direção ao assassino e fiz o meu disparo. O desastre foi como pensado. O revolver explodiu em minha mão com um som ensurdecedor. Projéteis se espalharam pela cabana, me atingindo e deixando o homem ileso. Talvez tenha sido a pior ideia que já tive e que me custou o resto da minha vida.");
	final10->itens = criar_vetor_itens(2);
	append_vetor_itens(final10->itens,sinalizador);
	append_vetor_itens(final10->itens,municao);
	
	Final *final11 = criar_final(11,"Sinalizador + chave","Não havia como escapar correndo daquela situação. Eu tinha que alcançar o meu carro, mas havia alguém entre mim e o veículo. Eu tinha de criar alguma distração. Numa noite tão escura, cegá-lo poderia ser minha melhor opção. Alcancei o meu sinalizador e a chave do carro. Ouvi a porta ceder. À primeira vista do sujeito, fiz o disparo. A cabana foi preenchida por uma luz e fumaça cegantes. Fiz a minha escolha: avancei em direção ao assassino. Ambos fomos ao chão no meio daquela luz vermelha ofuscante. Me levantei o mais rápido que pude e fui em direção ao carro. Ouvi disparos. O homem estava armado e eu fui atingido. Ainda que tenha conseguido entrar no carro, a dor me fez perder os sentidos e a direção. Foi assim que eu morri.");
	final11->itens = criar_vetor_itens(2);
	append_vetor_itens(final11->itens,sinalizador);
	append_vetor_itens(final11->itens,chave);
	
	Final *final12 = criar_final(12,"Sinalizador + rede","Um sinalizador serve unicamente para duas coisas: pedir ajuda ou iniciar uma chama. Com a entrada iminente do homem, uma das duas opções seria a minha escolha. Com uma rede em mãos, imobilizar o sujeito poderia me dar algum tempo. Em poucos instantes, ouço a porta ceder. Preparado para a entrada do homem, atiro a rede em sua direção. Com tudo muito escuro, a única reação do sujeito foi disparar para onde pudesse. Os tiros acertam ao redor da cabana e passam raspando por mim. Em seguida vem o meu: um único disparo que ilumina a cabana em direção ao assassino. O projétil atinge o alvo, as faíscas incendeiam o homem, que coberto pela rede não consegue extinguir. Uma armadilha fatal.");
	final12->itens = criar_vetor_itens(2);
	append_vetor_itens(final12->itens,sinalizador);
	append_vetor_itens(final12->itens,rede);
	
	Final *final13 = criar_final(13,"Sinalizador + dinamite","Se para escapar disso eu precisasse levar tudo abaixo, que fosse assim. Era questão de tempo até o sujeito estar aqui dentro e seria recebido da melhor forma possível. Deixei uma única dinamite logo na porta e me afastei. Com o sinalizador em mãos, esperei a entrada do homem. A porta cedeu. No mesmo instante, disparei o sinalizador contra a dinamite. Achei que fosse uma boa ideia. O projétil não foi suficiente para que qualquer coisa acontecesse. O assassino não teve piedade.");
	final13->itens = criar_vetor_itens(2);
	append_vetor_itens(final13->itens,sinalizador);
	append_vetor_itens(final13->itens,rede);
	
	Final *final14 = criar_final(14,"Sinalizador + armadilha","Uma armadilha tão indiscreta como essa serviria somente para um cego cair. E seria exatamente essa a minha saída. A coloquei onde o sujeito entraria, em poucos instantes o sujeito estaria aqui dentro. O homem entrou. Ele estava armado. Mas aposto que não esperava ser recebido com um disparo de um sinalizador. Fumaça e luz preencheram a cabana, cegando ambos. Ouvi um som que ouvia muito comumente durante as minhas caçadas: o estalo de uma armadilha sendo disparada. Quando a luz e a fumaça baixaram, lá estava uma presa genuína. Alguém escolheu a toca errada");
	final14->itens = criar_vetor_itens(2);
	append_vetor_itens(final14->itens,sinalizador);
	append_vetor_itens(final14->itens,armadilha);
	
	Final *final15 = criar_final(15,"Machado + gasolina","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");
	final15->itens = criar_vetor_itens(2);
	append_vetor_itens(final15->itens,machado);
	append_vetor_itens(final15->itens,gasolina);
	
	Final *final16 = criar_final(16,"Machado + arma","Como um bom caçador, minha prioridade era empunhar meu rifle, inseparável. O que quer que estivesse lá fora, seria recebido da melhor forma possível. O sujeito finalmente entrou. Ele estava armado como esperado. Antes que ele pudesse mexer um dedo, fiz o meu disparo. Não houve som, não houve recuo e não houve reação. Meu rifle estava vazio. Um erro que nunca havia cometido e que pagaria com a minha vida. Tentei alcançar o meu machado, a única arma que me sobrou, mas era inútil. A vantagem da distância foi minha condenação.");
	final16->itens = criar_vetor_itens(2);
	append_vetor_itens(final16->itens,machado);
	append_vetor_itens(final16->itens,arma);
	
	Final *final17 = criar_final(17,"Machado + munição","Eu sabia que só havia uma coisa a se fazer. Minha primeira reação foi recorrer a um tiro em quem quer que estivesse lá fora. Encontrei a munição, mas antes que eu pudesse alcançar o rifle, a porta cedeu. Ele estava armado, eu não. Meu machado era a coisa mais próxima a mim, mas a distância não me favorecia. Não havia como escapar do destino");
	final17->itens = criar_vetor_itens(2);
	append_vetor_itens(final17->itens,machado);
	append_vetor_itens(final17->itens,municao);
	
	Final *final18 = criar_final(18,"Machado + rede","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");
	final18->itens = criar_vetor_itens(2);
	append_vetor_itens(final18->itens,machado);
	append_vetor_itens(final18->itens,rede);
	
	Final *final19 = criar_final(19,"Gasolina + arma","Explodir meu caminho para fora de uma situação me parece razoável quando não há coisa ao meu redor que possa me salvar. Um tanque de combustível e uma bala é tudo o que preciso para acabar com quem quer que esteja do outro lado da porta. Rapidamente coloco o galão ali e sem esperar que a porta abra, preparo o meu tiro. Faço o disparo ao mesmo tempo em que a porta é aberta. Não há som ou recuo. A arma estava vazia. Como pude cometer um erro tão grotesco? Antes que tenha tempo de me lamentar, sou recebido com disparos em minha direção.");
	final19->itens = criar_vetor_itens(2);
	append_vetor_itens(final19->itens,gasolina);
	append_vetor_itens(final19->itens,arma);
	
	Final *final20 = criar_final(20,"Gasolina + munição","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");
	final20->itens = criar_vetor_itens(2);
	append_vetor_itens(final20->itens,gasolina);
	append_vetor_itens(final20->itens,municao);
	
	Final *final21 = criar_final(21,"Gasolina + rede","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");
	final21->itens = criar_vetor_itens(2);
	append_vetor_itens(final21->itens,gasolina);
	append_vetor_itens(final21->itens,rede);
	
	Final *final22 = criar_final(22,"Munição + armadilha","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");
	final22->itens = criar_vetor_itens(2);
	append_vetor_itens(final22->itens,armadilha);
	append_vetor_itens(final22->itens,municao);
	
	Final *final23 = criar_final(23,"Munição + rede","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");	
	final23->itens = criar_vetor_itens(2);
	append_vetor_itens(final23->itens,rede);
	append_vetor_itens(final23->itens,municao);
	
	Final *final_generico = criar_final(23,"generico","Os poucos instantes entre as batidas e a entrada do homem me pegaram desprevenidos. Tentei pegar as primeiras coisas que vi pela frente, mas por fim, só ocupei minhas mãos sem que pudesse fazer nada. A porta cedeu e em segundos vieram os disparos. Foi o meu fim.");	
	
	Saida *saida0 = criarSaida(0,"porta",920,200,150,400);
//	Saida *saida1 = criarSaida(0,"janela",50,300,750,300);
	
	saida0->finais = criar_vetor_finais(24);
//	saida1->finais = criar_vetor_finais(6);
	
	append_vetor_finais(saida0->finais,final0);
	append_vetor_finais(saida0->finais,final1);
	append_vetor_finais(saida0->finais,final3);
	append_vetor_finais(saida0->finais,final4);
	append_vetor_finais(saida0->finais,final6);
	append_vetor_finais(saida0->finais,final8);
	append_vetor_finais(saida0->finais,final9);
	append_vetor_finais(saida0->finais,final10);
	append_vetor_finais(saida0->finais,final11);
	append_vetor_finais(saida0->finais,final12);
	append_vetor_finais(saida0->finais,final13);
	append_vetor_finais(saida0->finais,final14);
	append_vetor_finais(saida0->finais,final15);
	append_vetor_finais(saida0->finais,final16);
	append_vetor_finais(saida0->finais,final17);
	append_vetor_finais(saida0->finais,final18);
	append_vetor_finais(saida0->finais,final19);
	append_vetor_finais(saida0->finais,final20);
	append_vetor_finais(saida0->finais,final21);
	append_vetor_finais(saida0->finais,final22);
	append_vetor_finais(saida0->finais,final23);
	
	void *img_cam0 = load_image(".\\Itens\\camera\\cena1.bmp",LarTela,AltTela,0,0);
	void *img_cam1 = load_image(".\\Itens\\camera\\cena2.bmp",LarTela,AltTela,0,0);
	void *img_cam2 = load_image(".\\Itens\\camera\\cena3.bmp",LarTela,AltTela,0,0);
	void *img_cam3 = load_image(".\\Itens\\camera\\cena4.bmp",LarTela,AltTela,0,0);
	
	TCamera *camera0 = criarCamera(0,img_cam0);
	camera0->saida = saida0;
	
	TCamera *camera1 = criarCamera(1,img_cam1);
	
	TCamera *camera2 = criarCamera(2,img_cam2);
	
	TCamera *camera3 = criarCamera(3,img_cam3);
 	
 	TInventario *inventario = criar_inventario(LarTela-100,0);
 	inventario->itens = criar_vetor_itens(2);
 	
 	TCamera *cameras = (TCamera*)malloc(sizeof(TCamera)*4);
 	
 	cameras[0] = *camera0;
 	cameras[1] = *camera1;
 	cameras[2] = *camera2;
 	cameras[3] = *camera3;
 	
 	cameras[0].itens = criar_vetor_itens(20);
 	cameras[1].itens = criar_vetor_itens(20);
 	cameras[2].itens = criar_vetor_itens(20);
 	cameras[3].itens = criar_vetor_itens(20);
 	
 	append_vetor_itens(cameras[0].itens,fosforo);
 	append_vetor_itens(cameras[0].itens,gasolina);
 	append_vetor_itens(cameras[2].itens,machado);
 	append_vetor_itens(cameras[2].itens,sinalizador);
 	append_vetor_itens(cameras[1].itens,chave);
 	append_vetor_itens(cameras[3].itens,rede);
	append_vetor_itens(cameras[1].itens,armadilha);
	append_vetor_itens(cameras[3].itens,arma);
	append_vetor_itens(cameras[3].itens,corda);
	
 	while(true) {
 		gt2 = GetTickCount();
 		
 		if(gt2 - gt1 > 1000/60) {
 			if(pg == 1) pg = 2; else pg = 1;
 			setvisualpage(pg);
 			cleardevice();
 			
 			mudarDeCamera(&camera_atual,&tecla,gt2,&tc2,&mudou);

 			mostrarCamera(&cameras[camera_atual]);
			mostrarItensCamera(&cameras[camera_atual]);
			
			mostrarInventario(inventario,hud_inventario);
			mostrarItensInventario(inventario);
			mostrarSaidasCamera(&cameras[camera_atual]);
			mostraTempo(tempo,hud_tempo,hud_tempo_mask);
			executaSom(&ts1);
			colisaoMouseItens(cameras[camera_atual],inventario);
			colisaoMouseSaidas(cameras[camera_atual],inventario,*final_generico);
			
			if(((gt2 - tempo) /1000) > 15) {
				Conclusao(Final(*final6));
			}
			
 			setactivepage(pg);
		}
		
	}
	
	deleteImage(img_cam0);
	deleteImage(img_cam1);
	deleteImage(img_cam2);
	deleteImage(img_cam3);

	
	deleteImage(item1_img);
	deleteImage(item1_mask);
	
	apaga_vetor_itens(&final0->itens);
	
	apaga_vetor_finais(&saida0->finais);
	free(saida0);
	apaga_vetor_itens(&cameras[0].itens);
 	free(cameras);
	return 0;
}

int Tutorial(){
	int pg = 1;
	int LarTela,AltTela;
	
	LarTela = 1280;
	AltTela = 720;
	
	void *botao1_img = load_image(".\\Hud\\Voltar.bmp",208,84,0,0);
	void *botao1_mask = load_image(".\\Hud\\VoltarWB.bmp",208,84,0,0);
	
	Botao *botao_voltar = criar_botao("voltar",botao1_img,botao1_mask,50,600,208,84);
	
	BotoesVetor *botoes = criar_vetor_botoes(1);
	
	append_vetor_botoes(botoes,botao_voltar);
	
	void *img_menu = load_image("Tutorial.bmp",LarTela,AltTela,0,0);
	
//	Botao *botao_voltar = criar_botao("voltar",0,0,30,AltTela - 50,100,50);
	
	unsigned long long int gt2;
	
	while(true){
		
		gt2 = GetTickCount();
		if(gt2 - gt1 > 1000/60) {
			if(pg == 1) pg = 2; else pg = 1;

	 		setvisualpage(pg);	
			cleardevice();
	
			putimage(0,0,img_menu,COPY_PUT);
			mostrarBotoes(botoes);
			colisaoMouseBotao(botoes);

			setactivepage(pg);		
		}
	}
	
	deleteImage(img_menu);
	return 0;
}

int Menu(){

	int pg;
	int LarTela,AltTela;
    
    LarTela = 1280;
	AltTela = 720;
	
	mciSendString("open .\\Audios\\MusicaTema.mp3 type MPEGVideo alias Tema", NULL, 0, 0);
	waveOutSetVolume(0,0xFFFFFFFF);
	mciSendString("play Tema repeat", NULL, 0, 0);
    
    void *botao1_img = load_image(".\\Hud\\Iniciar1.bmp",206,74,0,0);
	void *botao1_mask = load_image(".\\Hud\\Iniciar1WB.bmp",206,74,0,0);
    
    void *botao2_img = load_image(".\\Hud\\Iniciar2.bmp",100,50,0,0);
	void *botao2_mask = load_image(".\\Hud\\Iniciar2WB.bmp",100,50,0,0);
	
	void *botao3_img = load_image(".\\Hud\\Instrução1.bmp",330,80,0,0);
	void *botao3_mask = load_image(".\\Hud\\Instrução1WB.bmp",330,80,0,0);
	
	void *botao4_img = load_image(".\\Hud\\Sair1.bmp",142,74,0,0);
	void *botao4_mask = load_image(".\\Hud\\Sair1WB.bmp",142,74,0,0);
    
    Botao *botao_iniciar = criar_botao("iniciar",botao1_img,botao1_mask,LarTela/2 - 206/2,AltTela/2 + 105,206,74);

    Botao *botao_intro = criar_botao("intro",botao3_img,botao3_mask,LarTela/2 - 330/2,AltTela/2 +190,330,80); 

    Botao *botao_sair = criar_botao("sair",botao4_img,botao4_mask,LarTela/2 - 142/2,AltTela/2+280,142,74); 
	
	BotoesVetor *botoes = criar_vetor_botoes(3);
	
	append_vetor_botoes(botoes,botao_iniciar);
	append_vetor_botoes(botoes,botao_intro);
	append_vetor_botoes(botoes,botao_sair);

    void *img_menu = load_image("HorrorHut.bmp",LarTela,AltTela,0,0);

	unsigned long long int gt2;
	
	POINT P;
	HWND janela;
	janela = GetForegroundWindow();
	
	while(true){
		gt2 = GetTickCount();
		if(gt2 - gt1 > 1000/60) {
			if(pg == 1) pg = 2; else pg = 1;
	 		setvisualpage(pg);
	 		cleardevice();
			putimage(0,0,img_menu,COPY_PUT);
			mostrarBotoes(botoes);
			colisaoMouseBotao(botoes);
			setactivepage(pg);
		}
	}
	
	deleteImage(botao1_img);
	deleteImage(botao2_img);
	deleteImage(botao3_img);
	deleteImage(botao4_img);
	
	deleteImage(botao1_mask);
	deleteImage(botao2_mask);
	deleteImage(botao3_mask);
	deleteImage(botao4_mask);
	
	apaga_vetor_botoes(&botoes);
	return 0;
}

int main() {
	int pg = 1;
	int LarTela = 1280;
	int AltTela = 720;
	
	mciSendString("open .\\Audios\\crow-sound.mp3 type MPEGVideo alias Corvo", NULL, 0, 0);
	
	setlocale(LC_ALL,"Portuguese");
	initwindow(LarTela, AltTela,"meu jogo");
	if(pg == 1) pg = 2; else pg = 1;
	setvisualpage(pg);
	cleardevice();
  	//animacao_splash();
	setactivepage(pg);
	Menu();

 	closegraph();
	return 0;
}
