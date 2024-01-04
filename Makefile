all:
	g++ main.cpp -o main -g -I. `pkg-config --cflags --libs freetype2` -lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL -lGLU -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL

