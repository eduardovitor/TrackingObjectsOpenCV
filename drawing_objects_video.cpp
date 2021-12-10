#include <stdio.h> 
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>

using namespace std;

using namespace cv;


VideoCapture vc;
int thresh = 100;
RNG rng(12345);

//funções para calcular a mediana de um determinado número de frames do vídeo

int computeMedian(vector<int> elements) {
  nth_element(elements.begin(), elements.begin()+elements.size()/2, elements.end());
  return elements[elements.size()/2];
}

Mat compute_median(vector<Mat> vec) {
  Mat medianImg(vec[0].rows, vec[0].cols, CV_8UC3, Scalar(0, 0, 0));

  for(int row=0; row<vec[0].rows; row++) {
    for(int col=0; col<vec[0].cols; col++) 
    {
      vector<int> elements_B;
      vector<int> elements_G;
      vector<int> elements_R;

      for(int imgNumber=0; imgNumber<vec.size(); imgNumber++) 
      {
        int B = vec[imgNumber].at<Vec3b>(row, col)[0];
        int G = vec[imgNumber].at<Vec3b>(row, col)[1];
        int R = vec[imgNumber].at<Vec3b>(row, col)[2];

        elements_B.push_back(B);
        elements_G.push_back(G);
        elements_R.push_back(R);
      }
      medianImg.at<Vec3b>(row, col)[0]= computeMedian(elements_B);
      medianImg.at<Vec3b>(row, col)[1]= computeMedian(elements_G);
      medianImg.at<Vec3b>(row, col)[2]= computeMedian(elements_R);
    }
  }
  return medianImg;
}

Mat calculaFundo(){
	vc.open("BallMoving_5min.mp4");
	if (!vc.isOpened()) {
		printf("\n Arquivo nao encontrado");
	}
	//Escolhe 25 frames aleatórios para calcular o fundo do vídeo (ou seja a parte estástica do mesmo)
	default_random_engine generator;
	uniform_int_distribution<int>distribution(0, vc.get(CAP_PROP_FRAME_COUNT)); //pega ids respectivos dos frames
	vector<Mat> frames;
	Mat frame;
  	for(int i=0; i<25; i++)  {
	    int fid = distribution(generator);
	    vc.set(CAP_PROP_POS_FRAMES, fid);
	    Mat frame;
	    vc.read(frame); 
	    if(frame.empty())
	      continue;
	    frames.push_back(frame); //armazena cada um dos frames no vector frames
  	}
  	Mat medianFrame = compute_median(frames); //calcula a mediana dos frames, isto é, o fundo do vídeo
	vc.set(CAP_PROP_POS_FRAMES, 0); //reseta o id dos frames para o início (0)
	Mat grayMedianFrame;
	cvtColor(medianFrame, grayMedianFrame, COLOR_BGR2GRAY); //converte o fundo para tons de cinza
	vc.release();
	return grayMedianFrame;
}

void recebeFundo_desenhaContorno(Mat fundo_cinza){
	vc.open("BallMoving_5min.mp4");
	Mat frame;
	while(1) 
	{
	  vc.read(frame);
	  if (frame.empty())
	    break;
	  //Converte o frame atual para tons de cinza
	  cvtColor(frame, frame, COLOR_BGR2GRAY);
	  // Calcula a diferença do fundo e do frame atual, dessa forma detectando o movimento do objeto
	  Mat dframe;
	  absdiff(frame, fundo_cinza, dframe);
	  // Uso da função threshold transformando o frame em binário para facilitar a detecção do objeto
	  threshold(dframe, dframe, 30, 255, THRESH_BINARY);
	  // Detecta os contornos do objeto usando a função Canny e cv2.CHAIN_APPROX_NONE
	  Mat canny_output;
	  Canny( dframe, canny_output, thresh, thresh*2 );
	  vector<vector<Point>> contours;
	  vector<Vec4i> hierarchy;
	  findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	  Point a(35,35); //Definindo uma posição diferente para o contorno
      for( size_t i = 0; i< contours.size(); i++ ) {
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours(frame, contours, (int)i, color, 8, LINE_8, hierarchy, 0, a);
    }
    imshow("Video", frame);
	int key=waitKey(20);
	if(key=='q'){
		break;
	}
  }
  vc.release();
}

int main(int argc, char const *argv[]) {
	Mat fundo_cinza=calculaFundo();
 	recebeFundo_desenhaContorno(fundo_cinza);
}