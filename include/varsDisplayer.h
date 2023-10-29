struct detected_obj{
	int cm_x;
	int cm_y;
	char obj_name[50];
};

void open_rt_database(void);
void found_object(int cm_x, int cm_y);
void found_ball(int cm_x, int cm_y);
void found_landamark(int cm_x, int cm_y);
