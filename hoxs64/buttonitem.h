#pragma once

struct ButtonItem
{
	typedef enum tagButtonItemOption
	{
		None,
		SingleButton,
		MultipleButton,
		SingleAxis,
		AllButtons,
		AllAxis
	} ButtonItemOption;

	ButtonItem();
	ButtonItem(ButtonItemOption option);
	ButtonItem(ButtonItemOption option, const DIDEVICEOBJECTINSTANCE& objectInfo);

	ButtonItemOption option;
	DIDEVICEOBJECTINSTANCE objectInfo;	
};