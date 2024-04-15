//5a 22 XX XX YY YY   设置云台角度 单位0.1° 俯仰角范围-20~90°， 偏航角范围0~360°   例：*(int16_t*)&BUF[2]=目标偏航角  *(int16_t*)&BUF[4]=目标俯仰角 



*(int16_t*)(&tcp_tx_buf[55]) = YUNTAI.shijipianhangjiao*10;	//偏航角
*(int16_t*)(&tcp_tx_buf[57]) = YUNTAI.shijifuyangjiao*10;	//俯仰角

没有传感器的信息参数

qt.multimedia.audioresampler: Resampling failed -1072875851
