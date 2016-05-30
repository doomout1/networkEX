const MAXAVGSIZE  = 200;

/*
 double 형의 데이터를 받을 수 있는데, 최대 200개까지 받을 수 있다.
*/
struct input_data {
  double input_data<MAXAVGSIZE>;
};

typedef struct input_data input_data;

/*
프로그램의 이름과 버전을 정의 한다. 프로그램의 이름은 서비스의 이름이기도 하다
프로그램의 이름은 AVERAGEPROG이고, 프로그램의 이름을 가리키는 번호는 3901을 사용했다.
프로그램 버전은 1이다.
클라이언트에서 서버에 요청하기 위해서 호출하는 함수의 이름은 AVERAGE이다. 나중에 개발자는 이 함 수를 개발해야 하는데, 함수의 실제 이름은 ‘average_버전번호’가 된다. 그러므로 여기에서 함수 이름은 average_1이 된다. 이 함수는 하나의 매개 변수를 가진다. 나중에 이 매개 변수로 평균계산에 사용할 값의 배열을 넘길 것이다.
*/
program AVERAGEPROG {
    version AVERAGEVERS {
        double AVERAGE(input_data) = 1;
    } = 1;
} = 3901;

