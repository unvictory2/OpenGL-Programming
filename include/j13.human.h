//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		2020. 10. 12					//
//////////////////////////////////////////

#ifndef HUMAN_H
#define HUMAN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Bones in a Human skeleton
// 뼈 정의, pelvis 골반이 기준. 
// 좌/우는 보는 입장에서가 아니라 사람 입장에서 좌우임. (즉 내 화면상은 반대)
enum Human_Bone {		// 20 bones
    pelvis, spine, 
	neck, head, 
	clavicleL, upperarmL, forearmL, handL, // hand는 palm/5 fingers로 나눌 수도 있다.
	clavicleR, upperarmR, forearmR, handR,
	thighL, calfL, footL, toeL, 
	thighR, calfR, footR, toeR
};

// several poses
// 모션 != 포즈. 모션은 움직임, 포즈는 움직임의 결과물. 관절의 회전 각도를 코드로 넣으면 그 사이 모션은 보간돼서 알아서 생성됨
// 포즈 추가 시 밑의 POSENUM 늘리기
enum Human_Pose {
	// base는 기본 차렷 포즈
	start, walkLeftUpMid, 
	walkLeftUpHigh, walkLeftDownMid, walkLeftDownHigh,
	Human_Pose_Count
};

// 재생 순서. 별로 좋은 방법은 아닌 것 같지만 간단하고 빠르게 하기 위함
const Human_Pose poseSequence[] = {
	start,
	walkLeftUpMid, walkLeftUpHigh, walkLeftUpMid, 
	start,
	walkLeftDownMid, walkLeftDownHigh, walkLeftDownMid, start
};
const int poseSequenceCount = sizeof(poseSequence) / sizeof(Human_Pose);


// Default values
const int BONENUM = 20;
const int POSENUM = 5;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Human
{
public:
    // Skeleton Attributes
   glm::quat	BoneRotate[BONENUM]; // 각 뼈가 얼마나 회전했는지.

    // Constructor 
    Human()
    {
		// bone length 뼈마다 길이 다르니까. 팔이랑 손바닥이 같은 길이는 아니니까.
		BoneLength[pelvis] = 1.0f;		BoneLength[spine] = 3.0f;
		BoneLength[neck] = 1.0f;		BoneLength[head] = 1.0f;
		BoneLength[clavicleL] = 1.0f;	BoneLength[upperarmL] = 2.0f;	BoneLength[forearmL] = 1.5f;	BoneLength[handL] = 1.0f;
		BoneLength[clavicleR] = 1.0f;	BoneLength[upperarmR] = 2.0f;	BoneLength[forearmR] = 1.5f;	BoneLength[handR] = 1.0f;
		BoneLength[thighL] = 2.5f;		BoneLength[calfL] = 2.0f;		BoneLength[footL] = 1.0f;		BoneLength[toeL] = 0.5f;
		BoneLength[thighR] = 2.5f;		BoneLength[calfR] = 2.0f;		BoneLength[footR] = 1.0f;		BoneLength[toeR] = 0.5f;

		// setup poses
		SetupPoses();
		// bone rotation - base pose
		SetPose(start);
	}

	// 포즈 바꾸기
	void SetPose(Human_Pose pose) // 포즈 - 각 뼈에 대한 회전을 가지고 있음
	{
		for (int i = pelvis; i < toeR; i++) // 각 뼈에 대해
			BoneRotate[i] = Pose[pose][i]; // 현재 뼈의 회전을 포즈가 가지고 있는 해당 뼈의 회전으로 바꿈
	}

	// 특정 bone에 quertanion 줘서 회전시키기
	void SetBoneRotation(Human_Bone bone, glm::quat q)
	{
		BoneRotate[bone] = q;
	}

	// 키프레임 변화 (보간)
	void MixPose(Human_Pose p, Human_Pose q, float t)
	{
		for (int i = pelvis; i < toeR; i++) // 모든 뼈에 대해
			BoneRotate[i] = glm::mix(Pose[p][i], Pose[q][i], t); // p라는 포즈의 i라는 bone에서 q라는 포즈의 i라는 bone으로 시간 t에 따라서 mix
	}

	void DrawHuman(Shader shader, unsigned int cubeVAO, glm::mat4 model)
	{
		// 밖에 있는 model matrix 가져옴. 월드에서 캐릭터가 움직일 수 있게 하기 위함 
		glm::mat4 bone = model;
		glm::mat4 mpelvis = model;
		glm::mat4 mspine;

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// draw pelvis
		bone = bone * glm::mat4_cast(BoneRotate[pelvis]); // pelvis 회전
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[pelvis], 1.0f)); // pelvis scale
		shader.setMat4("model", bone); // pelvis 그리기
		shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f); // pelvis 색깔 칠하기
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//1.0f / BoneLength[pelvis] 하는 이유 : scale 조절 matrix 원래대로 돌려놓기. 안 그러면 계속 누적됨.  
		// translation은 계속 이전 위치 기준으로 움직일 거라 초기화 안 함. rotation은 부모 관절 돌아가면 같이 돌아가니까 얘도 냅둠. scale만 초기화.
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[pelvis], 1.0f)); 
		// draw spine 척추
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[pelvis], 0.0f)); // pelvis가 기준이니까 pelvis 높이만큼 위로 올라가서 척추 그리기 
		bone = bone * glm::mat4_cast(BoneRotate[spine]); // 회전 있다면 bone update
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[spine], 1.0f)); // spine 크기로 스케일링
		shader.setMat4("model", bone); // 척추 그리기
		shader.setVec3("objectColor", 1.0f, 0.5f, 0.3f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[spine], 1.0f)); // scale matrix 초기화
		mspine = bone;
		// draw neck
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[spine], 0.0f)); // 척추 위에 목 그리기
		bone = bone * glm::mat4_cast(BoneRotate[neck]); // spine과 별도로 neck이 돌아갔다면, 즉 목만 돌아갔다면 적용
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[neck], 0.5f)); // 목 크기로 그리기
		shader.setMat4("model", bone); // 그리기
		shader.setVec3("objectColor", 0.0f, 0.5f, 1.0f); // 색
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[neck], 2.0f));
		// draw head
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[neck], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[head]);
		bone = glm::scale(bone, glm::vec3(1.0f, BoneLength[head], 1.0f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.5f, 0.5f, 1.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(1.0f, 1.0f / BoneLength[head], 1.0f));
		// draw clavicleL
		bone = mspine;
		bone = glm::translate(bone, glm::vec3(0.2f, BoneLength[spine], 0.0f));
		bone = glm::rotate(bone, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)); // 쇄골은 실제 인체와 같이 땅과 평행하게
		bone = bone * glm::mat4_cast(BoneRotate[clavicleL]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[clavicleL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[clavicleL], 2.0f));
		// draw upperarmL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[clavicleL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[upperarmL]);
		bone = glm::rotate(bone, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)); // 쇄골 돌아가서 팔 회전 
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[upperarmL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[upperarmL], 2.0f));
		// draw forearmL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[forearmL]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[forearmL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[forearmL], 2.0f));
		// draw handL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[handL]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[handL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[handL], 2.0f));
		// 이전 부모와 연결 안 된 경우
		// draw clavicleR
		bone = mspine; // 척추에서부터
		bone = glm::translate(bone, glm::vec3(-.2f, BoneLength[spine], 0.0f));
		bone = glm::rotate(bone, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)); // 쇄골은 실제 인체와 같이 땅과 평행하게
		bone = bone * glm::mat4_cast(BoneRotate[clavicleR]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[clavicleR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[clavicleR], 2.0f));
		// draw upperarmR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[clavicleR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[upperarmR]);
		bone = glm::rotate(bone, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)); // 쇄골 돌아가서 팔 회조ㅓㄴ
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[upperarmR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.0f, 0.7f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[upperarmR], 2.0f));
		// draw forearmR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[upperarmR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[forearmR]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[forearmR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.7f, 0.0f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[forearmR], 2.0f));
		// draw handR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[forearmR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[handR]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[handR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.0f, 0.5f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[handR], 2.0f));
		// 골반 아래 ================================
		// draw thighL
		bone = mpelvis;
		//bone = glm::translate(bone, glm::vec3(0.5f, -BoneLength[pelvis], 0.0f));
		bone = glm::translate(bone, glm::vec3(0.5f, 0.0f, 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[thighL]);
		bone = glm::rotate(bone, glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)); // 골반 아래부터는 아래로 뒤집기
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[thighL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 1.0f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[thighL], 2.0f));
		// draw calfL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[thighL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[calfL]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[calfL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.5f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[calfL], 2.0f));
		// draw footL
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[calfL], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[footL]);
		bone = glm::rotate(bone, glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));  //발은 땅 바라보게 추가로 회전
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[footL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.9f, 0.2f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[footL], 2.0f));
		// draw toeL
		bone = glm::translate(bone, glm::vec3(-0.0f, BoneLength[footL], 0.0f));
		//bone = bone * glm::mat4_cast(BoneRotate[toeL]); //발가락은 회전할 일 x
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[toeL], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.1f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[toeL], 2.0f));
		// draw thighR
		bone = mpelvis;
		bone = glm::translate(bone, glm::vec3(-0.5f, 0.0f, 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[thighR]);
		bone = glm::rotate(bone, glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)); // 골반 아래부터는 아래로 뒤집기
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[thighR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 1.0f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[thighR], 2.0f));
		// draw calfR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[thighR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[calfR]);
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[calfR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.5f, 0.7f, 0.0f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[calfR], 2.0f));
		// draw footR
		bone = glm::translate(bone, glm::vec3(0.0f, BoneLength[calfR], 0.0f));
		bone = bone * glm::mat4_cast(BoneRotate[footR]);
		bone = glm::rotate(bone, glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));  //발은 땅 바라보게 추가로 회전
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[footR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.9f, 0.2f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[footR], 2.0f));
		// draw toeR
		bone = glm::translate(bone, glm::vec3(-0.0f, BoneLength[footR], 0.0f));
		bone = glm::scale(bone, glm::vec3(0.5f, BoneLength[toeR], 0.5f));
		shader.setMat4("model", bone);
		shader.setVec3("objectColor", 0.3f, 0.1f, 0.5f);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		bone = glm::scale(bone, glm::vec3(2.0f, 1.0f / BoneLength[toeR], 2.0f));
		 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

private:
	float		BoneLength[BONENUM];
	glm::quat	Pose[POSENUM][BONENUM];

	// 뼈 이름 참고
	//enum Human_Bone {		// 20 bones
	//	pelvis, spine,
	//	neck, head,
	//	clavicleL, upperarmL, forearmL, handL, // hand는 palm/5 fingers로 나눌 수도 있다.
	//	clavicleR, upperarmR, forearmR, handR,
	//	thighL, calfL, footL, toeL,
	//	thighR, calfR, footR, toeR
	//};

	// x축: 카메라 방향
	// y축: 상하
	// z축: 좌우


	// 초기화. 포즈 전부 읽어들이기. 
	void SetupPoses()
	{
		for (int p = 0; p < POSENUM; p++)
		{
			for (int i = 0; i < BONENUM; i++)
				// Pose는 quertanion 2차 행렬. p에는 포즈 이름, i에는 bone들 들어가있음. 일단 초기화.
				// 이건 z축으로 돼있는데, 이건 팔만 올리는 거라 걸을 땐 축이 다르다. x축이다. 그래서 걷는 걸 만들 땐 x축 기준으로 바꿔야 할지도.
				Pose[p][i] = glm::angleAxis(glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

			switch (p) {
			case start:// 최종 포즈 기준으로 입력. 차렷포즈. 쇄골은 상완이랑 직각임(현실 인체에서 그러니까)
				break;
			case walkLeftUpMid:// 왼팔/오른발 살짝 앞, 오른팔/왼발 살짝 뒤
				Pose[walkLeftUpMid][upperarmL] = glm::angleAxis(glm::radians(-20.f), glm::vec3(0.f, 1.f, 0.f));
				Pose[walkLeftUpMid][upperarmR] = glm::angleAxis(glm::radians(-20.f), glm::vec3(0.f, 1.f, 0.f));

				// 상완은 쇄골 때문에 회전해서 y축 회전이 카메라 방향인데, 보통은 x축이 카메라 방향이다. 
				Pose[walkLeftUpMid][forearmL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpMid][forearmR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				// 하체
				Pose[walkLeftUpMid][thighL] = glm::angleAxis(glm::radians(12.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpMid][calfL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpMid][footL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				Pose[walkLeftUpMid][thighR] = glm::angleAxis(glm::radians(-12.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpMid][calfR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpMid][footR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));


				break;
			case walkLeftUpHigh:
				Pose[walkLeftUpHigh][upperarmL] = glm::angleAxis(glm::radians(-25.f), glm::vec3(0.f, 1.f, 0.f));
				Pose[walkLeftUpHigh][upperarmR] = glm::angleAxis(glm::radians(-22.f), glm::vec3(0.f, 1.f, 0.f));

				Pose[walkLeftUpHigh][forearmL] = glm::angleAxis(glm::radians(20.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpHigh][forearmR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				// 하체
				Pose[walkLeftUpHigh][thighL] = glm::angleAxis(glm::radians(25.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpHigh][calfL] = glm::angleAxis(glm::radians(15.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpHigh][footL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				Pose[walkLeftUpHigh][thighR] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpHigh][calfR] = glm::angleAxis(glm::radians(15.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftUpHigh][footR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				break;
			case walkLeftDownMid:
				Pose[walkLeftDownMid][upperarmL] = glm::angleAxis(glm::radians(20.f), glm::vec3(0.f, 1.f, 0.f));
				Pose[walkLeftDownMid][upperarmR] = glm::angleAxis(glm::radians(20.f), glm::vec3(0.f, 1.f, 0.f));

				// 상완은 쇄골 때문에 회전해서 y축 회전이 카메라 방향인데, 보통은 x축이 카메라 방향이다. 
				Pose[walkLeftDownMid][forearmL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownMid][forearmR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				// 하체
				Pose[walkLeftDownMid][thighL] = glm::angleAxis(glm::radians(-12.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownMid][calfL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownMid][footL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				Pose[walkLeftDownMid][thighR] = glm::angleAxis(glm::radians(12.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownMid][calfR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownMid][footR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				break;			
			case walkLeftDownHigh:
				Pose[walkLeftDownHigh][upperarmL] = glm::angleAxis(glm::radians(22.f), glm::vec3(0.f, 1.f, 0.f));
				Pose[walkLeftDownHigh][upperarmR] = glm::angleAxis(glm::radians(25.f), glm::vec3(0.f, 1.f, 0.f));

				Pose[walkLeftDownHigh][forearmL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownHigh][forearmR] = glm::angleAxis(glm::radians(20.f), glm::vec3(1.f, 0.f, 0.f));

				// 하체
				Pose[walkLeftDownHigh][thighL] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownHigh][calfL] = glm::angleAxis(glm::radians(15.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownHigh][footL] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));

				Pose[walkLeftDownHigh][thighR] = glm::angleAxis(glm::radians(25.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownHigh][calfR] = glm::angleAxis(glm::radians(15.f), glm::vec3(1.f, 0.f, 0.f));
				Pose[walkLeftDownHigh][footR] = glm::angleAxis(glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
				break;
			default:
				;
			};
		}
	}

};
#endif