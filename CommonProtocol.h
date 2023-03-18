
enum en_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & Server Protocol
	//
	////////////////////////////////////////////////////////


	//=========================================================================================
	//=========================================================================================
	// Game Server
	//=========================================================================================
	//=========================================================================================
	en_PACKET_CS_GAME_SERVER = 0,

	//------------------------------------------------------------
	// 게임 서버 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_LOGIN,

	//------------------------------------------------------------
	// 게임 서버 로그인 실패 응답
	//
	//	{
	//		WORD	Type
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SC_GAME_RES_LOGIN_FAILED,

	//------------------------------------------------------------
	// 본인/타인 캐릭터 생성
	//
	//	{
	//		WORD	Type
	// 
	//		INT64	AccountNo
	// 
	//		Vector3 Location
	// 
	//		int		Level
	//		int		MaxHP
	//		int		AttackDamage
	// 
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CREATE_MY_CHARACTER,
	en_PACKET_SC_GAME_CREATE_NEW_CHARACTER,


	//------------------------------------------------------------
	// 본인 캐릭터 앞,뒤 이동 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	//		Vector3	Direction
	//		float	AxisValue
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_MOVE_FORWARD,

	//------------------------------------------------------------
	// 타 세션 캐릭터 앞,뒤 이동 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//	
	//		int64	AccountNo	
	// 
	//		Vector3	Direction
	//		float	AxisValue
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_MOVE_FORWARD,

	//------------------------------------------------------------
	// 본인 캐릭터 좌,우 이동 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	//		Vector3	Direction
	//		float	AxisValue
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_MOVE_RIGHT,


	//------------------------------------------------------------
	// 타 세션 캐릭터 좌,우 이동 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//			
	//		int64	AccountNo	
	// 
	//		Vector3	Direction
	//		float	AxisValue
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_MOVE_RIGHT,


	//------------------------------------------------------------
	// 본인 캐릭터 이동 정지 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	//		Vector3	Location
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_STOP_MOVE,


	//------------------------------------------------------------
	// 타 세션 캐릭터 이동 정지 알림(Server -> Client)
	// {
	//		WORD	Type
	// 
	//		INT64	AccountNo
	//		
	//		Vector3	Location
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_STOP_MOVE,


	//------------------------------------------------------------
	// 본인 캐릭터 점프 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_JUMP,

	//------------------------------------------------------------
	// 타 세션 캐릭터 점프 알림(Server -> Client)
	// 
	// {
	//		WORD	Type
	//		
	//		INT64	AccountNo
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_JUMP,

	//------------------------------------------------------------
	// 본인 캐릭터 회전 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	//		Vector3 Rotation
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_TURN,

	//------------------------------------------------------------
	// 타 세션 캐릭터 회전 알림(Server -> Client)
	// {
	//		WORD	Type
	// 
	//		INT64	AccountNo
	//		
	//		Vector3 Rotation
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_TURN,

	//------------------------------------------------------------
	// 본인 캐릭터 뛰기 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_SPRINT,

	//------------------------------------------------------------
	// 타 세션 캐릭터 뛰기 알림(Server -> Client)
	// {
	//		WORD	Type
	// 
	//		INT64	AccountNo
	//		
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_SPRINT,

	//------------------------------------------------------------
	// 본인 캐릭터 걷기 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_SPRINT_END,

	//------------------------------------------------------------
	// 타 세션 캐릭터 걷기 알림(Server -> Client)
	// {
	//		WORD	Type
	// 
	//		INT64	AccountNo
	//		
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_SPRINT_END,

	//------------------------------------------------------------
	// 본인 캐릭터 공격 알림(Client -> Server)
	// 
	// {
	//		WORD	Type
	//		
	//		Vector3 Location
	//		Vector3 Rotation
	// 
	//		int64	HitAccountNo
	// }
	//------------------------------------------------------------
	en_PACKET_CS_GAME_CHARACTER_ATTACK,

	//------------------------------------------------------------
	// 캐릭터 공격 알림(Server -> Client)
	// 
	// {
	//		WORD	Type
	//		
	//		int64	AttackAccountNo
	//		int64	HitAccountNo
	//		int		AttackDamage
	// }
	//------------------------------------------------------------
	en_PACKET_SC_GAME_CHARACTER_ATTACK,





	//=========================================================================================
	//=========================================================================================
	// Login Server
	//=========================================================================================
	//=========================================================================================
	en_PACKET_CS_LOGIN_SERVER = 1000,

	//------------------------------------------------------------
	// 로그인 서버로 클라이언트 계정 등록 요청
	//
	//	{
	//		WORD	Type
	//
	//		char	ID[40]
	//		char	PW[64]				// Hashed Password(sha256)
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_LOGIN_REQ_REGIST,


	//------------------------------------------------------------
	// 로그인 서버로 클라이언트 계정 등록 성공
	//
	//	{
	//		WORD	Type
	//		
	//		char	isSuccess (0 = fail, 1 = success)
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_LOGIN_RES_REGIST,

	//------------------------------------------------------------
	// 로그인 서버로 클라이언트 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		char	ID[40]
	//		char	PW[64]				// Hashed Password(sha256)
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_LOGIN_REQ_LOGIN,

	//------------------------------------------------------------
	// 로그인 서버에서 클라이언트로 로그인 성공
	//
	//	{
	//		WORD	Type
	//
	//		char	isSuccess (0 = fail, 1 = success) // isSuccess = 0이라면 아래는 보내지않음
	//		INT64	AccountNo
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SC_LOGIN_RES_LOGIN,

};



enum en_PACKET_SC_RES_FAILURE_REASON
{
	dfLOGIN_RES_FAILURE_NONE = 0,		// 미인증상태
	dfLOGIN_RES_FAILURE_GAME,			// 게임중
	dfLOGIN_RES_FAILURE_ID_PW_MISMATCH,	// ID/PW가 일치하지 않음
};


enum en_PACKET_CS_GAME_RES_LOGIN
{
	dfGAME_LOGIN_FAIL = 0,			// 세션키 오류 또는 Account 데이블상의 오류
	dfGAME_LOGIN_SUCCESS,			// 성공
	dfGAME_LOGIN_NONE_OF_TOKEN,		// 로그인 인증 토큰 부재
};
