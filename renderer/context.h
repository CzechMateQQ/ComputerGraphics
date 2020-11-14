#pragma once

class context
{
	// Forward declaration
	struct GLFWwindow *window;

public:
	bool init(int width, int height, const char *title);
	void tick();
	void term();   // terminate
	void clear();

	bool shouldClose() const;

	float time() const;
	void resetTime(float resetValue);
};

