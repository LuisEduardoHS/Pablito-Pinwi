#ifndef _Camara_
#define _Camara_

class Camara
{
private:
	bool thirdPerson;
	D3DXVECTOR3 lookDirection;

public:
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Target;
	D3DXVECTOR3 Up;

	D3DXMATRIX View;
	D3DXMATRIX Projection;

	D3DXVECTOR3 DefaultForward;
	D3DXVECTOR3 DefaultRight;
	D3DXVECTOR3 Forward;
	D3DXVECTOR3 Right;

	D3DXMATRIX camRotationMatrix;
	D3DXMATRIX groundWorld;

	float Yaw;
	float Pitch;

	Camara(D3DXVECTOR3 Position, D3DXVECTOR3 Target, D3DXVECTOR3 Up, int Width, int Height)
	{
		this->Position = Position;
		this->Target = Target;
		this->Up = Up;

		//Set the View matrix 
		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);

		//Set the Projection matrix
		D3DXMatrixPerspectiveFovLH(&Projection, 
			D3DXToRadian(45), (float)Width / Height, 
			0.1f, 2500.0f);

		DefaultForward = D3DXVECTOR3(0.0f,0.0f,1.0f);
		DefaultRight = D3DXVECTOR3(1.0f,0.0f,0.0f);
		Forward = D3DXVECTOR3(0.0f,0.0f,1.0f);
		Right = D3DXVECTOR3(1.0f,0.0f,0.0f);

		Yaw = 0.0f;
		Pitch = 0.0f;
		thirdPerson = true;
		lookDirection = DefaultForward;
	}

	bool IsThirdPerson() const { return thirdPerson; }
	void TogglePerspective() { thirdPerson = !thirdPerson; }

	// Rotation and movement are independent of the camera's follow offset.
	void UpdateRotation(float yaw, float pitch)
	{
		Yaw += yaw;
		Pitch += pitch;

		// Evita cruzar la vertical e invertir la vista. Se limita el angulo
		// acumulado para responder de inmediato al cambiar de direccion.
		const float pitchLimit = D3DXToRadian(85.0f);
		if (Pitch > pitchLimit)
			Pitch = pitchLimit;
		else if (Pitch < -pitchLimit)
			Pitch = -pitchLimit;

		D3DXMatrixRotationYawPitchRoll(&camRotationMatrix, Yaw, Pitch, 0);
		D3DXVec3TransformNormal(&lookDirection, &DefaultForward, &camRotationMatrix);
		D3DXVec3Normalize(&lookDirection, &lookDirection);

		D3DXMATRIX RotateYTempMatrix;
		D3DXMatrixRotationY(&RotateYTempMatrix, Yaw);

		D3DXVec3TransformCoord(&Right, &DefaultRight, &RotateYTempMatrix);
		Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3TransformCoord(&Forward, &DefaultForward, &RotateYTempMatrix);
	}

	void FollowPlayer(const D3DXVECTOR3& eyePosition, float followDistance)
	{
		Position = eyePosition;
		if (thirdPerson)
			Position -= lookDirection * followDistance;
		UpdateView();
	}

	void UpdateView()
	{
		Target = Position + lookDirection;
		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);
	}

	void setSkyCamara(Camara *camara)
	{
		D3DXMatrixRotationYawPitchRoll(&camRotationMatrix, camara->Yaw, camara->Pitch, 0);
		D3DXVec3TransformCoord(&Target, &DefaultForward, &camRotationMatrix);
		D3DXVec3Normalize(&Target, &Target);

		Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);
	}

	D3DXVECTOR2 getXZ(){
		return D3DXVECTOR2(Position.x, Position.z);
	}
};

#endif
