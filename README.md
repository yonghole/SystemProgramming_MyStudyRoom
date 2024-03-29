# 나만의 독서실

System / Embedded Programming Project

시연 영상 >> [https://www.youtube.com/watch?v=j0Dk9-GmF0s](https://www.youtube.com/watch?v=j0Dk9-GmF0s)

![Untitled](https://user-images.githubusercontent.com/55180768/122497011-ae35de80-d027-11eb-92ed-b2aec0302518.png)

# 1. 개요

최근 코로나 바이러스로 인해 외부 활동이 제한되고, 다중 이용 시설에 대한 주의가 각별해지고 있습니다. 평소 다양한 이유로 집 외의 학습 환경(카페, 스터디 카페, 독서실 등) 을 이용하던 대학생들은 이런 상황에 대처하여 새로운 학습 환경을 모색해야 할 처지가 되었습니다. 또한, 위의 이유로 인해 집 안에서 학습하는 시간이 증가하며, 집 안에서 학습을 진행할 때 최대한 집중력을 향상할 수 있는 환경을 만드는 것이 중요해졌습니다.

팀원 및 주변 지인들을 통해 평소 학생들이 집 외의 다른 학습 환경을 찾는 이유는 다음과 같았습니다.

1) 익숙한 환경에 계속 쉬게 된다.

2) 눈치 볼 사람이 없어 계속해서 자리를 이탈한다.

3) 휴식과 공부 공간의 분리가 없다.

4) 가족 구성원의 방해

5) 동기 부여 부족 등

위 원인들을 통해 집에서 효율적으로 공부를 하기 위해서는 1) 평소 휴식 공간으로 사용하던 곳을 학습에 적합한 환경으로 변경해야 하고, 2) 자리를 이탈하지 않도록 동기부여를 해줄 수 있는 방안이 필요하며 3) 동시에 가족 구성원들에게 학습 중임을 인지시켜 방해를 사전 차단하는 등의 조치가 필요함을 알 수 있었습니다. 

‘나만의 독서실'은 이 모든 과정을 다양한 센서들의 상호 작용과 적절한 제어 및 모니터링을 통해 자동화하여, 사용자가 코로나 바이러스로부터 안전하면서도 효율적인 학습 공간을 구성할 수 있도록 합니다.

# 제공 기능

본 시스템에서 제공하는 기능은 다음과 같습니다.

1. 적정 온습도 유지

    온습도 센서를 활용하여 공부에 집중할 수 있는 최적화된 온도를 자동으로
    유지합니다.

2. 적정 산소 농도 유지

    방 안의 산소 농도를 계속해서 측정하고, 특정 농도 이하일 경우 산소 발생기
    혹은 창문 개방을 통해 자동으로 공부에 최적화된 산소 농도를 유지합니다.

3. 공부 시간 표시

    압력센서를 활용하여 사용자가 의자에 앉은 경우를 공부 중인 것으로
    판단하고, 자동으로 공부 시간을 측정하고 표시합니다.

4. 가족 구성원에게 ‘공부 중' 표시

    압력 센서가 눌린 경우 공부 중이라는 상태를 가족 구성원들에게 표시하여
    방해를 줄일 수 있도록 합니다.

5. 제어 시스템 제공

    스위치를 통해 ‘나만의 독서실' 서비스를 On/Off 할 수 있으며, 버튼을 통해
    측정된 공부 시간을 초기화 할 수 있습니다.

# 시스템 개요

<img width="1021" alt="_2021-06-18__10 27 47" src="https://user-images.githubusercontent.com/55180768/122496986-a5dda380-d027-11eb-8d46-cb4809bb2413.png">

< 센서 Pi - 관리자 영역 >

주된 목적이 센싱이기 때문에, 센서들과 가장 가까운 곳에 위치시키고, 센싱
알고리즘의 변경에 의한 센싱오류 등의 문제를 방지하기 위해 관리자
영역으로 설정합니다.

1. 온습도 센서를 활용하여 온도, 습도가 미리 설정한 기준을 벗어날 경우
메인 파이로 신호를 전달합니다.
2. 압력 센서를 통해 사용자의 착석 여부를 확인하여 신호를 전송합니다.
3. 산소 센서를 이용하여 실내 산소 농도를 측정하고, 일정 기준 이하일

    경우 메인 파이로 신호를 보냅니다.

< 메인 Pi - 사용자 영역 >

메인 Pi 의 주된 목적은 센싱값에 따른 엑츄레이터의 동작여부를 결정 하는
것입니다.
만약 사용자가 필요하다면, 제한된 기능에 내에서 UI 를 통해 수정을
허용합니다.

(임계값,엑츄레이터 동작 단계 등) 하지만 그 부분을 제외한 기능은
관리자영역으로 설정하여 메인 알고리즘을 보호합니다. 시스템의 일부이긴
하지만, 사용자의 접근을 허용하기 때문에 사용자영역으로 구분합니다.

1. 센서 Pi 로부터 전달 받은 신호를 수신하고 분석합니다.
2. 수신한 신호가 적정 범위에 있는지를 확인합니다.
3. 신호가 적정 범위를 벗어난 경우 이를 제어하기 위해 Actuator Pi 에

    신호를 전송합니다.

< Actuator Pi - 관리자 영역>

- 주된 목적이 엑츄레이터 작동이기 때문에, 엑츄레이터들과 가장 가까운
곳에 위치시키고, 동작 알고리즘의 변경에 의한 동작 오류 등의 문제를
방지하기 위해 관리자 영역으로 설정합니다.
1. 메인 Pi 로부터 제어 신호를 수신합니다.
2. 수신한 신호에 맞는 센서에 적정 신호를 송출하여 LCD, LED,

    서보모터를 비롯한 액추에이터를 작동합니다.

# 시스템 작동 시나리오

1. 스위치를 통해 시스템을 ON 합니다. (공부를 시작하기 위함)

2. 각종 센서들이 동작하기 시작하며 문제 발생 여부를 확인합니다.

3. 사용자가 의자에 앉으면 압력센서 인식하여 방 문에 붙여진 LED가 ON되고

앉은 시간을 계산하여 LCD 에 나타냅니다.

4. 센서들이 문제 발생 여부를 확인합니다.

4-1. 적절한 온습도의 범위를 벗어난다면 적절한 온습도를 유지하기 위해
온도조절기와 습도조절기가 작동합니다.

4-2. 산소 센서를 통해 적정 산소 농도 범위를 벗어나면 적정 산소 범위가 될
때까지 창문을 개방합니다.

4-3. 조도 센서를 통해 적정 조도를 유지하기 위해 조명을 작동시킵니다.

5. 시스템이 더 이상 필요하지 않을 때 스위치를 통해 OFF합니다.

# 사용 센서 목록

<img width="778" alt="스크린샷 2021-06-18 오전 11 24 01" src="https://user-images.githubusercontent.com/55180768/122497020-b0983880-d027-11eb-8f2c-530962faddca.png">

# 시스템 구성

<img width="1020" alt="_2021-06-18__10 43 11" src="https://user-images.githubusercontent.com/55180768/122497000-aa09c100-d027-11eb-9c28-8fde84465d1a.png">

# 실제 구현

<img width="909" alt="_2021-06-18__11 05 39" src="https://user-images.githubusercontent.com/55180768/122497005-ad9d4800-d027-11eb-8df4-c7607f840287.png">

<img width="909" alt="_2021-06-18__11 01 53" src="https://user-images.githubusercontent.com/55180768/122497003-ab3aee00-d027-11eb-8939-1fdec8ee554a.png">



# References

- DHT11(온습도센서) 데이터시트:
https:/www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
- Reference Python Code for i2c:
https://bitbucket.org/MattHawkinsUK/rpispy-misc/raw/master/python/lcd_i2c.py
- Reference C Code for i2c bus opening :
https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface
