//#pragma once
//
//struct ButtonItem
//{
//	typedef enum tagButtonItemOption
//	{
//		None,
//		SingleButton,
//		MultipleButton,
//		SingleAxis,
//		AllButtons,
//	} ButtonItemOption;
//
//	typedef enum tagAxisDirection
//	{
//		DirectionAny,
//		DirectionMin,
//		DirectionMax,
//	} AxisDirection;
//
//	typedef enum tagObjectType : int
//	{
//		ObjectTypeButton = 1,
//		ObjectTypeAxis = 2,
//		ObjectTypeButtonAndAxis = 3,
//	} ObjectType;
//
//	ButtonItem(ButtonItemOption option);
//	ButtonItem(ButtonItemOption option, AxisDirection direction);
//	ButtonItem(ButtonItemOption option, AxisDirection direction, const DIDEVICEOBJECTINSTANCE& objectInfo);
//
//	ButtonItemOption option;
//	AxisDirection direction;
//	DIDEVICEOBJECTINSTANCE objectInfo;	
//};