　　void Float2Byte(float floatNum,unsigned char* byteArry)
　　{
　　 char* pchar=(char*)&floatNum;
　　 for(int i=0;i<sizeof(float);i++)
　　 {
　　 *byteArry=*pchar;
　　 pchar++;
　　 byteArry++;
　　 }
　　}
　　