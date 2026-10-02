# 작업 기록: 포스팅 4 AI NPC 업데이트 빈도 (태스크 13, 태스크 11의 세 번째 반복)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다. 지금의 상태는 [STATUS.md](../STATUS.md)에 있다.

## 2026-10-02 태스크 11.1\~11.4: 업데이트 빈도 측정과 재측정 (`update-frequency`\~`update-frequency3`, `dormancy3`\~`dormancy6`)

세 실행의 CSV 값과 중앙값, 변동 폭은 [STATUS.md](../STATUS.md)의 "측정 결과" 표에 있다.

NPC 업데이트 빈도 `update-frequency2`(2026-10-02, `SetNetUpdateFrequency(10.f)`, 중앙값 실행 `r3`, 본체 화면 3840×2160에서 측정)는 `netflush_avg_ms`와 `out_bytes_per_sec_per_conn`이 `dormancy2`와 구별된다(변화 1.647 > 변동 폭 1.476, 1,908 > 586). `work_avg_ms`(변화 1.814 < `dormancy2`의 변동 폭 2.032)와 `work_p99_ms`(3.018 < 3.294)는 이 측정으로는 차이를 구별하지 못했다. 앞선 `update-frequency`는 `r2`의 측정 시작 15초 뒤 클라이언트 선호도가 다시 설정되어 실패했고 수치를 쓰지 않는다. 재측정(사용자 결정, 2026-10-02): 직전 구성을 `SetNetUpdateFrequency` 한 줄을 되돌린 빌드로 본체 화면에서 다시 쟀다(`dormancy6`, 중앙값 실행 `r1`). `dormancy3`\~`dormancy5`는 측정 중 선호도 재설정으로 실패해 수치를 쓰지 않는다. 이어서 한 줄을 다시 넣은 빌드로 `update-frequency3`(중앙값 실행 `r1`)을 쟀다. 포스팅 4의 비교는 같은 화면 조건에서 연달아 잰 `dormancy6`과 `update-frequency3`으로 한다. `netflush_avg_ms`(변화 0.631 > 변동 폭 0.488), `work_p99_ms`(1.524 > 1.029), `out_bytes_per_sec_per_conn`(1,917 > 9)은 구별되고, `work_avg_ms`(0.449 < 0.513)는 다시 재도 구별되지 않아 그대로 결과로 삼는다. `dormancy2`와 `dormancy6`의 `work_avg_ms` 차이 1.066은 코드가 같은 두 묶음의 차이다(화면 조건이 달랐다. 원인은 확인하지 않음).

## 2026-10-02 태스크 11.5\~11.10: 포스팅 4 시각 자료와 완료 (`visual6`\~`visual10`)

태스크 1\~13을 끝냈고 포스팅 1\~4가 완료다(포스팅 4는 2026-10-02, 태그 `post-04-update-frequency`). 세 기법을 모두 적용했다. 빌드된 바이너리는 main의 소스(`SetNetUpdateFrequency(10.f)` 적용, 시연용 NPC 코드 포함)와 같다.

- 현재 포스팅: 포스팅 4(AI NPC 업데이트 빈도)가 끝났다(2026-10-02, 태그 `post-04-update-frequency`). 단기에 구현하는 기법은 이것이 마지막이다. 비교한 묶음은 `dormancy6`(중앙값 실행 `r1`)과 `update-frequency3`(`r1`)이다. 다음 시각 자료 라벨은 `visual11`이다.
- 끝낸 단계: 11.1\~11.10(2026-10-02). `NetUpdateFrequency` 값 10, 재측정, 관찰과 선택의 방향은 사용자가 정했다. Insights 값(서버 프레임 시간 평균 13.60 → 13.16ms로 구별되지 않음, P99 20.84 → 19.50ms, 리플리케이션 시간 9.35 → 8.73ms, `Connection 0` 대역폭 2,793 → 1,303바이트/초, NPC 하나의 갱신 간격 4패킷 약 134ms)은 [포스팅 4](../../Posts/04-update-frequency/README.md)와 [후보 기법 자료](../../Posts/04-update-frequency/candidates.md)에 있다.
- 영상: 시연용 NPC(`c8e4323`, `-LabShowcaseNpc`)를 넣어 `visual9-r1`(한 줄을 되돌린 빌드)과 `visual10-r1`(적용 빌드)에서 0번 창을 60fps로 8초씩 찍었다(둘 다 종료 코드 0). `before-npc.gif`, `after-npc.gif`는 그 원본을 줄이지 않고 잘라 4배 느리게 만든 것이고, 프레임마다 읽은 NPC의 화면 위치는 바뀐 간격 평균 46.4ms → 130.6ms, 한 번에 3.7px → 14.1px(중앙값)이다. 사용자가 느린 GIF와 실제 속도 원본 모두에서 끊김이 보인다고 확인했다. `visual7`, `visual8`, `showcase-off1`은 시연용 NPC의 작은 규모 확인이다(인자가 없으면 NPC 수와 채널 수가 그대로). 앞서 `run-manual.ps1`로 찍은 24fps 영상과 `after-clip.gif`(`visual6-r1`)에서는 끊김을 구별하지 못했다.
- `run-manual.ps1`의 관찰자 카메라는 커서의 창 안 x좌표에 따라 돈다(조작 도구 좌표로 1px에 약 0.3°, 창 폭만큼만 돌릴 수 있다). 에이전트의 `Stop-Process`가 권한 분류기에서 거부되어, 수동 실행은 사용자가 그 창에서 Enter를 눌러 종료했다.
- **포스팅 4 "관찰"과 "선택"(태스크 11.8).** [포스팅 4](../../Posts/04-update-frequency/README.md)의 "관찰"과 "선택"은 인터뷰 답(대역폭 중심, 포스팅 1의 이유와 전제가 채워졌는지 확인)을 에이전트가 문장으로 옮긴 것이다. 초안과 끊김 영상은 승인됐다(2026-10-02).
