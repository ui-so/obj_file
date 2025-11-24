#define _CRT_SECURE_NO_WARNINGS 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random> 
#include <ctime> 

#define M_PI 3.141592
#define MAX_LINE_LENGTH 256

std::default_random_engine dre((unsigned int)time(NULL));


//--- 필요한헤더파일선언
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

typedef struct {
	float x, y, z;
} Vertex;
typedef struct {
	// v1, v2, v3 대신 배열 사용
	unsigned int v_indices[3]; // 정점 인덱스 3개
	unsigned int n_indices[3]; // 법선 인덱스 3개 (★추가)
} Face;
typedef struct {
	Vertex* vertices;
	size_t vertex_count;
	Vertex* normals;     // ★ 법선 배열 (추가됨)
	size_t normal_count; // ★ 법선 개수 (추가됨)
	Face* faces;
	size_t face_count;
} Model;
void read_newline(char* str) {
	char* pos;
	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}
void read_obj_file(const char* filename, Model* model) {
	FILE* file;
	fopen_s(&file, filename, "r");
	if (!file) { /* ... (기존 오류 처리) ... */ return; }

	char line[MAX_LINE_LENGTH];
	model->vertex_count = 0;
	model->face_count = 0;
	model->normal_count = 0; // ★ 법선 카운트 초기화

	// --- 1. 첫 번째 패스: 개수 세기 ---
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'v' && line[1] == ' ')
			model->vertex_count++;
		else if (line[0] == 'v' && line[1] == 'n') // ★ 'vn' 개수 세기
			model->normal_count++;
		else if (line[0] == 'f' && line[1] == ' ')
			model->face_count++;
	}

	// --- 2. 메모리 할당 ---
	fseek(file, 0, SEEK_SET);
	model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
	model->normals = (Vertex*)malloc(model->normal_count * sizeof(Vertex)); // ★ 법선 배열 할당
	model->faces = (Face*)malloc(model->face_count * sizeof(Face));

	size_t vertex_index = 0, face_index = 0, normal_index = 0; // ★ normal_index 추가

	// --- 3. 두 번째 패스: 데이터 읽기 ---
	while (fgets(line, sizeof(line), file)) {
		read_newline(line);
		if (line[0] == 'v' && line[1] == ' ') {
			// (정점 읽기)
			sscanf_s(line + 2, "%f %f %f",
				&model->vertices[vertex_index].x,
				&model->vertices[vertex_index].y,
				&model->vertices[vertex_index].z);
			vertex_index++;
		}
		else if (line[0] == 'v' && line[1] == 'n') { // ★ 'vn' 읽기
			sscanf_s(line + 3, "%f %f %f", &model->normals[normal_index].x,
				&model->normals[normal_index].y,
				&model->normals[normal_index].z);
			normal_index++;
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			unsigned int v[3], n[3];
			int result = -1;

			// ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
			//
			//      이 부분을 수정했습니다!
			//      (f v/vt/n 형식을 파싱)
			//
			result = sscanf_s(line + 2, "%u/%*u/%u %u/%*u/%u %u/%*u/%u",
				&v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);
			//
			// ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★

			if (result < 6) {
				// (v/vt/n 형식이 아닌 다른 형식이면 건너뜀)
				continue;
			}

			// ★ Face 구조체에 저장 (OBJ는 1-based index이므로 -1)
			for (int i = 0; i < 3; i++) {
				model->faces[face_index].v_indices[i] = v[i] - 1;
				model->faces[face_index].n_indices[i] = n[i] - 1;
			}
			face_index++;
		}
	}
	model->face_count = face_index; // 실제 읽은 면 개수
	fclose(file);
}

struct S
{
	float x;
	float y;
	float z;

	float speed;

	int count;
};

// 육면체
float vertices[] = {
	// 위치              // 법선 (면이 바라보는 방향)
	// 뒷면 (z = -0.5)
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	// 앞면 (z = 0.5)
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	// 왼쪽 면 (x = -0.5)
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	// 오른쪽 면 (x = 0.5)
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	 // 아랫면 (y = -0.5)
	 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	 // 윗면 (y = 0.5)
	 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

// 사각뿔
float Vertices[] = {
	// 위치 (x, y, z)        // 법선 (nx, ny, nz)

	// 밑면 (y = -0.5, 법선: 0, -1, 0)
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	// 앞면 (법선: 0, 0.447, 0.894)
	 0.0f,  0.5f,  0.0f,  0.0f, 0.447f, 0.894f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.447f, 0.894f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.447f, 0.894f,

	 // 오른쪽 면 (법선: 0.894, 0.447, 0)
	  0.0f,  0.5f,  0.0f,  0.894f, 0.447f, 0.0f,
	  0.5f, -0.5f,  0.5f,  0.894f, 0.447f, 0.0f,
	  0.5f, -0.5f, -0.5f,  0.894f, 0.447f, 0.0f,

	  // 뒷면 (법선: 0, 0.447, -0.894)
	   0.0f,  0.5f,  0.0f,  0.0f, 0.447f, -0.894f,
	   0.5f, -0.5f, -0.5f,  0.0f, 0.447f, -0.894f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 0.447f, -0.894f,

	  // 왼쪽 면 (법선: -0.894, 0.447, 0)
	   0.0f,  0.5f,  0.0f, -0.894f, 0.447f, 0.0f,
	  -0.5f, -0.5f, -0.5f, -0.894f, 0.447f, 0.0f,
	  -0.5f, -0.5f,  0.5f, -0.894f, 0.447f, 0.0f
};

static int VertexCount = 36;

static float Light_Power[3] = {};
static float Light_rotate = 0.0f;
static float Light_move = 0.0f;
static int Light_flag = 0;


//--- 아래5개함수는사용자정의함수임
void make_vertexShaders();
void make_fragmentShaders();
void InitBuffer();
void InitModelBuffers();
void make_shaderProgram();

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLuint VAO, VBO_position;
GLuint orbitVAO, orbitVBO;

GLchar* vertexSource, * fragmentSource;
GLuint vertexShader, fragmentShader;
GLuint shaderProgramID;

static GLfloat triShape[6][3] = {};
static GLfloat colors[6][3] = {};
GLuint vao, vbo;

Model my_model;
GLuint model_VAO, model_VBO[2], model_EBO;

//--- 필요한변수선언
GLint width, height;

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void TimerFunction(int value)
{
	glutPostRedisplay(); // 화면 재 출력
	glutTimerFunc(50, TimerFunction, 1); // 타이머함수 재 설정
}


void main(int argc, char** argv)
//--- 윈도우출력하고콜백함수설정
{
	width = 800;
	height = 800;

	for (int i = 0; i < 3; i++)
	{
		Light_Power[i] = 1.0f;
	}

	//--- 윈도우생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow(" ");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	read_obj_file("office.obj", &my_model);
	if (my_model.vertex_count == 0) {
		std::cerr << "ERROR: Failed to load model or model is empty." << std::endl;
		// (필요시 여기서 프로그램을 종료할 수 있습니다)
	}
	else {
		printf("Loaded model: %zu vertices, %zu faces\n", my_model.vertex_count, my_model.face_count);
	}

	//--- 세이더읽어와서세이더프로그램만들기: 사용자정의함수호출
	make_shaderProgram();
	InitBuffer();
	InitModelBuffers();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);

	glutTimerFunc(50, TimerFunction, 1);
	glutMainLoop();
}




void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl");
	//--- 버텍스세이더객체만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더코드를세이더객체에넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- 버텍스세이더컴파일하기
	glCompileShader(vertexShader);
	//--- 컴파일이제대로되지않은경우 : 에러체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}

}

//--- 프래그먼트세이더객체만들기
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl");
	//--- 프래그먼트세이더객체만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더코드를세이더객체에넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- 프래그먼트세이더컴파일
	glCompileShader(fragmentShader);
	//--- 컴파일이제대로되지않은경우: 컴파일에러체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_shaderProgram()
{
	make_vertexShaders();
	make_fragmentShaders();
	//-- shader Program
	shaderProgramID = glCreateProgram();
	//--- 버텍스세이더만들기
   //--- 프래그먼트세이더만들기
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- 세이더삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}

// 육면체, 사각뿔
void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo); // VBO 1개만 생성

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgramID);
}

// obj 버퍼
void InitModelBuffers()
{
	if (my_model.vertex_count == 0 || my_model.normal_count == 0) return;

	size_t data_size = my_model.face_count * 3 * 6;
	GLfloat* interleaved_data = (GLfloat*)malloc(data_size * sizeof(GLfloat));

	size_t idx = 0;
	for (size_t i = 0; i < my_model.face_count; i++) 
	{
		Face f = my_model.faces[i];

		for (int j = 0; j < 3; j++) 
		{
			Vertex v = my_model.vertices[f.v_indices[j]];
			Vertex n = my_model.normals[f.n_indices[j]];

			interleaved_data[idx++] = v.x;
			interleaved_data[idx++] = v.y;
			interleaved_data[idx++] = v.z;

			interleaved_data[idx++] = n.x;
			interleaved_data[idx++] = n.y;
			interleaved_data[idx++] = n.z;
		}
	}

	glGenVertexArrays(1, &model_VAO);
	glBindVertexArray(model_VAO);

	glGenBuffers(1, &model_VBO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, model_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, data_size * sizeof(GLfloat), interleaved_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	free(interleaved_data);
}

GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// --- 1. 쉐이더 변수 위치 찾기 ---
	int modelLoc = glGetUniformLocation(shaderProgramID, "model"); // "model" 위치 가져오기
	int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	int projLoc = glGetUniformLocation(shaderProgramID, "projection");

	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	glUniform3f(lightColorLocation, Light_Power[0], Light_Power[1], Light_Power[2]);

	glm::vec4 baseLightPos = glm::vec4(0.0f, 0.5f, 1.05f + Light_move, 1.0f);
	glm::mat4 lightTransform = glm::mat4(1.0f);
	lightTransform = glm::rotate(lightTransform, glm::radians(Light_rotate), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 finalLightPos = lightTransform * baseLightPos;
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3f(lightPosLocation, finalLightPos.x, finalLightPos.y, finalLightPos.z);

	unsigned int lightambient = glGetUniformLocation(shaderProgramID, "ambientLight");
	glUniform1f(lightambient, 0.5f);

	// --- 2. 카메라 (View) 및 원근 (Projection) 행렬 설정 ---
	glm::mat4 vTransform = glm::lookAt(
		glm::vec3(0.0f, 5.0f, 5.0f),  // 카메라 위치
		glm::vec3(0.0f, 0.0f, 0.0f),  // 바라보는 지점
		glm::vec3(0.0f, 1.0f, 0.0f)   // 위쪽 방향
	);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);


	glm::mat4 pTransform = glm::perspective(glm::radians(30.0f), (float)width / (float)height, 0.1f, 200.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	// 5. --- 그리기 ---

	const int num_segments = 36;
	unsigned int colorLoc = glGetUniformLocation(shaderProgramID, "objectColor"); // objectColor 사용


	glUniform3f(objColorLocation, 0.5, 0.5, 0.5);
	if (my_model.face_count > 0)
	{
		glBindVertexArray(model_VAO);

		glm::mat4 model_transform = glm::mat4(1.0f);
		model_transform = glm::scale(model_transform, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_transform));

		glDrawArrays(GL_TRIANGLES, 0, my_model.face_count * 3);
	}

	glutSwapBuffers();
}


//--- 다시그리기콜백함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
	{
		glutLeaveMainLoop();
	}

	}
	glutPostRedisplay();
}