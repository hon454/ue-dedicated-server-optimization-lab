# 작업 기록: 포스팅 2 관련성과 컬 거리 (태스크 11, 첫 번째 반복)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다. 지금의 상태는 [STATUS.md](../STATUS.md)에 있다.

## 2026-10-02 태스크 11.3\~11.4: 관련성 측정과 비교 (`relevancy`, `relevancy2`)

관련성 `relevancy2`(2026-10-02, 태스크 11.3)는 두 클래스의 `bAlwaysRelevant = true`를 지운 빌드로 확정 명령에 `-Label relevancy2 -Runs 3`을 더해 실행했고 종료 코드 0, 선호도 재적용이 측정 시작 28\~29초 전에 끝났다. 앞선 `relevancy-r1`은 측정 끝 시각(13:43:59)에 클라이언트 선호도가 다시 설정되어 실패 처리됐고(`baseline-r2`와 같은 유형) 수치를 쓰지 않는다.

- **차이가 있다.** 모든 시간 지표의 중앙값 변화가 두 구성의 변동 폭 중 큰 쪽보다 크다(`work_avg_ms` 181.393 > 25.477, `netflush_avg_ms` 176.018 > 22.815, `out_bytes_per_sec_per_conn` 24,820 > 3,993). 포화는 없다.
- **틱 예산 안으로 들어와 `frames`가 실행마다 크게 다르다(1,304\~1,797).** 일한 시간은 거의 같고(`work_avg_ms` 변동 폭 0.193) 틱 속도 제한 대기가 다르다(Insights에서 프레임당 15.75\~28.41ms, [후보 기법 자료](../../Posts/02-relevancy/candidates.md) 1절). 초당 값인 `out_bytes_per_sec_per_conn`도 `frames`를 따라 흔들린다(3,871\~4,475). 대기가 실행마다 다른 이유는 모른다.
- **Insights의 "서버 프레임 시간"은 틱 속도 제한 대기를 뺀 값으로 읽는다([ADR-0010](../Decisions/0010-frame-time-without-tick-wait.md), 2026-10-02 승인).** `relevancy2`는 평균 17.60 / 17.76 / 17.64ms(중앙값 17.64, `r3`), P99 26.64 / 30.18 / 26.12ms(중앙값 26.64, `r1`)다. 옛 정의(구간 ÷ `Frame` Count)로는 33.38\~45.99ms였다. 포스팅 1과 README의 기준선 값도 새 정의로 바꿨다(평균 198.43, P99 262.96ms).
- 화면 글자는 `relevancy2-r3` 내려다보기 순번 02에서 `nodes=114 npcs=7`, 3인칭 순번 04에서 `nodes=80 npcs=5`다(예상 약 98, 약 6). 3인칭 04에서는 검증용 노드가 고갈된 때라 보이지 않고, 같은 순번의 `r1`에서는 서 있다. 순번 01(t=30s)의 `r3`에도 서 있다.

## 2026-10-02 태스크 11.5\~11.10: 포스팅 2 시각 자료와 완료 (`visual2`, `visual3`)

- 현재 포스팅: 포스팅 2(관련성과 컬 거리). 직전 구성은 `baseline3`이고 전후 자동 스크린샷 순번은 3인칭 04, 내려다보기 02다.
- 끝낸 단계: 11.1(코드), 11.2(빌드), 11.3(`relevancy2` 3회), 11.4(위 비교), 11.5(`Posts/02-relevancy/images/`의 `before-`/`after-` `tpp.png`, `topdown.png`, 직전 구성은 `baseline3-r1`, 이번은 중앙값 실행 `relevancy2-r3`), 11.7([후보 기법 자료](../../Posts/02-relevancy/candidates.md)), 11.6, 11.8(2026-10-02 인터뷰, 네 질문 모두 에이전트 추천을 고름. 초안을 사용자가 승인), 11.9, 11.10(커밋과 `post-02-relevancy` 태그). 포스팅 2가 끝났다.
- 11.6(2026-10-02): `visual2-r1`(지금 빌드)에서 `after-clip.gif`, 두 줄을 잠시 되돌린 빌드의 `visual3-r1`에서 `before-clip.gif`를 측정 구간에 10초씩 찍었다. 15fps로는 9.51MB, 13.26MB라 사용자 요청으로 폭 960px 그대로 8fps, 48색(`palettegen=max_colors=48`, `paletteuse=dither=bayer:bayer_scale=3:diff_mode=rectangle`)으로 다시 변환해 3.62MB, 4.49MB로 줄였다. 32색은 NPC의 빨간 점이 회색이 되어 버렸다. 미리보기에서 적용 후는 원 안의 점(화면 글자 노드 108\~115)이 플레이어를 따라 바뀌고, 적용 전은 `nodes=5001`로 화면 끝까지 점이 찬 것을 확인했다. 그 뒤 `git checkout -- Source`, 빌드 성공, 소스 변경 없음을 확인했다. 두 실행은 녹화 중 클라이언트 선호도 재적용으로 종료 코드 1이었고 수치는 쓰지 않는다. 영상의 빌드에는 `8075998`(채집 진행 표시)이 들어 있고 `relevancy2`의 측정 빌드(`b438852`)에는 없다. `after-timing.png`, `after-network.png`는 후보 캡처에 포스팅 1과 같은 자리의 상자를 그렸고(사용자 선택), `before-timing.png`, `before-network.png`는 포스팅 1의 `timing.png`, `network.png`를 복사했다.
