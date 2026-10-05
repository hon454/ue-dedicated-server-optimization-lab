# 현재 상태

마지막 갱신: 2026-10-06

## 단계

**단기 완료.** 태스크 1\~14 가운데 에이전트 몫을 모두 끝냈고(2026-10-02) 포스팅 0\~4가 완료다. 태그 `post-00-testbed`\~`post-04-update-frequency`가 원격에 있다(2026-10-02). 사용자의 14.5(전체 읽기)에서 나온 문제로 포스팅 0\~4와 루트 README를 새 틀로 다시 썼고, main을 `87c83f9`까지 푸시했다(2026-10-03). 태그 시점의 글은 옛 틀이다. 세 기법을 모두 적용했다. 경위는 [Worklog/00-testbed.md](Worklog/00-testbed.md)(태스크 1\~9, 보정 실행 표), [Worklog/01-baseline.md](Worklog/01-baseline.md)(태스크 10), [Worklog/02-relevancy.md](Worklog/02-relevancy.md)(태스크 11), [Worklog/03-dormancy.md](Worklog/03-dormancy.md)(태스크 12), [Worklog/04-update-frequency.md](Worklog/04-update-frequency.md)(태스크 13)에 있고, 태스크 9.4\~9.5와 14는 [Worklog/00-testbed.md](Worklog/00-testbed.md)의 "태스크 9.4\~9.5, 14"에 있다. 다음 작업에 영향을 주는 것만 여기에 남긴다.

- **2막 단계 1\~3(태스크 15\~21)이 끝났다(2026-10-04).** 기법 전환 인자, 확장 요소, 규모 확정, 포스팅 5(`post-05-expanded-testbed`)까지다. 확정값은 아래 "확정할 값"의 2막 줄, 인자는 "명령"에 있고, 경위는 [Worklog/05-expanded-testbed.md](Worklog/05-expanded-testbed.md)의 "태스크 15"부터 "태스크 15\~21: STATUS.md에서 옮긴 경위"까지에 있다.
- **태스크 22(포스팅 6, `post-06-three-techniques-again`)가 끝났다(2026-10-05).** 구성별 `work_avg_ms` 중앙값은 213.555 → 35.674 → 19.536 → 16.732다([포스팅 6 측정 기록](../Posts/06-three-techniques-again/measurements.md) 3절). 경위는 [Worklog/06-three-techniques-again.md](Worklog/06-three-techniques-again.md)의 "태스크 22"에 있다.
- **태스크 24(포스팅 7, `post-07-net-driver-breakdown`)가 끝났다(2026-10-06).** `GameNetDriver` Incl 가운데 `Prioritize Actors Time` 56.4%, `Consider Actors Time` 23.5%이고, 활성 목록 5,272개 가운데 자원 노드가 4,887개다. 경위는 [Worklog/07-net-driver-breakdown.md](Worklog/07-net-driver-breakdown.md)의 "태스크 24"에 있다.
- **태스크 25(포스팅 8, 자원 노드의 Net Update Frequency 낮추기)를 시작했다(2026-10-06).** 사용자가 [후보](../Posts/08-node-update-frequency/candidates.md) A를 골랐다. 구현과 작은 규모 확인이 끝났고, 빌드된 바이너리는 그 커밋의 소스다. 인자는 "명령"에 있다. 낮춘 구성에서는 채집과 되살아남이 `ForceNetUpdate()`로 다음 고려 시각을 당긴다(`LabResourceNode.cpp`의 `WakeForChange`). 새 서버 로그 `lab_consider_list avg_per_frame=`(측정 구간의 프레임당 Consider List 길이, 엔진 지표 `NumConsideredActors`)가 작은 규모에서 85.1 → 15.2였다(`tsmall-nodeuf-off1-r1`, `tsmall-nodeuf-on1-r1`, 클라이언트 2, 자원 노드 100). 낮춘 구성에서도 검증용 자원 노드가 고갈되어 화면에서 사라졌다.
- **시리즈 웹 페이지를 공개했다(2026-10-03).** [UE Dedicated Server, 단계별로 최적화해 보기](https://hon454.github.io/ue-dedicated-server-optimization-lab/)는 [Site/index.html](../Site/index.html) 한 파일이다. main의 `Site/`가 바뀐 채로 푸시되면 워크플로 `Deploy Pages`가 다시 올린다(첫 배포 18초, 실행 37113083287). 수치의 출처와 모형의 한계는 [Site/README.md](../Site/README.md)에 있다. 1막의 네 단계만 다루고, 2막의 구성은 [backlog.md](backlog.md)에 적었다.
- 다음 시각 자료 라벨은 `visual14`다(`visual13`은 README의 8개 창 화면, 2026-10-05).

## 다음 할 일

1. **[사람] 다시 쓴 포스팅 네 편을 읽는다.** 테스트베드, 기준선, Dormancy, Net Update Frequency 글의 "문제", "원리"와 기준선 글의 "선택"은 에이전트 초안이다(Relevancy 글과 루트 README의 틀은 승인됨). 고칠 곳이 나오면 에이전트가 고친다. 틀과 문장 규칙은 [posting.md](Guides/posting.md), 결정은 [ADR-0015](Decisions/0015-post-body-and-measurement-record.md), 경위는 [Worklog/00-testbed.md](Worklog/00-testbed.md)의 "태스크 14.5: 포스팅 다시 쓰기"에 있다. 이미 붙인 태그는 옮기지 않는다.
2. **[사람] 포스팅 8 초안 확인.** [본문](../Posts/08-node-update-frequency/README.md)과 [측정 기록](../Posts/08-node-update-frequency/measurements.md) 초안을 썼다(2026-10-06). "문제"와 "원리"는 에이전트 초안이라 승인이 필요하다. Insights 캡처는 측정 기록 4절에 넣었다. 남은 것: 태그 `post-08-node-update-frequency`.

## 포스팅 진행

| 포스팅 | 상태 | 태그 |
| --- | --- | --- |
| 0. 테스트베드와 측정 방법 | 완료 | `post-00-testbed` |
| 1. Always Relevant 기준선 | 완료 | `post-01-baseline` |
| 2. Relevancy와 Net Cull Distance(관련성) | 완료 | `post-02-relevancy` |
| 3. 자원 노드 Dormancy(휴면) | 완료 | `post-03-dormancy` |
| 4. AI NPC Net Update Frequency(업데이트 빈도) | 완료 | `post-04-update-frequency` |
| 5. 테스트베드 확장과 새 기준선 | 완료 | `post-05-expanded-testbed` |
| 6. 1막의 세 최적화를 2막에 다시 적용 | 완료 | `post-06-three-techniques-again` |
| 7. 네트워크 드라이버 자체 시간 나누기 | 완료 | `post-07-net-driver-breakdown` |
| 8. 자원 노드의 Net Update Frequency 낮추기 | 초안 | |

## 명령

태스크 8에서 확정했다(2026-10-01).

- 빌드: `powershell -ExecutionPolicy Bypass -File Scripts/build.ps1` (`UnrealEditor`가 떠 있으면 스크립트가 빌드를 거부한다. 에디터나 다른 체크아웃의 실행이 끝난 뒤에 한다)
- 시나리오 실행(확정 규모, `-Label`과 `-Runs` 없이): `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Clients 8 -Nodes 5000 -Npcs 300 -Warmup 30 -Measure 60`. 측정할 때는 `-Label <새 라벨> -Runs 3`을 더한다. 서버 마스크는 스크립트 기본값 252(논리 프로세서 2\~7)이고 트레이스는 켜진다. `calib-f-r1`이 이 조건의 실행이다.
- 작은 규모 확인용: `powershell -ExecutionPolicy Bypass -File Scripts/run-scenario.ps1 -Label <새 라벨> -Clients 2 -Nodes 100 -Npcs 10 -Warmup 20 -Measure 30 -NoTrace`
- 확정 규모 측정은 시작하기 전에 잴 구성, 라벨, 걸리는 시간, 화면 조건을 알리고 답을 받는다. 실행마다 서버 로그(`lab_config`, `lab_buildings`, `lab_npcs_near_players`, `lab_nodes_moved_from_harvest_spot`, 준비 구간의 `open_actor_channels_per_conn`)와 자동 스크린샷을 확인한다.
- 측정 중에는 클라이언트 창에 키 입력을 하지 않고, Insights 분석이나 빌드 같은 무거운 작업을 하지 않는다. 에이전트도 문서 편집을 포함해 다른 작업을 하지 않는다(`calib-g-r1` 실행 중의 문서 편집, [Worklog/00-testbed.md](Worklog/00-testbed.md) "태스크 9.1\~9.3"). 서버만 논리 프로세서 2\~7에 고정하므로 다른 프로그램은 그 코어를 쓸 수 있다. 에디터가 열려 있으면 스크립트가 실행을 거부한다.
- 구성 사이의 비교는 같은 화면 조건에서 연달아 잰 묶음끼리 한다. 코드가 같은 `dormancy2`(원격 데스크톱 화면)와 `dormancy6`(본체 화면)의 `work_avg_ms` 중앙값이 1.066 달랐다([Worklog/04-update-frequency.md](Worklog/04-update-frequency.md) "태스크 11.1\~11.4"). 화면 조건이 같아도 몇 시간 떨어진 묶음은 비교하지 않는다. 소스가 같은 `toggle1`과 두 시간 뒤의 `abcheck-old2`의 중앙값이 0.628 달랐다(13.581, 12.953). 지난 구성과 비교해야 하면 그 소스를 다시 빌드해 연달아 잰다(`git restore --source=<커밋> --worktree Source/`, 빌드, 측정, `git restore Source/`, 빌드. 그동안 다른 세션은 커밋하지 않는다).
- Insights: `powershell -ExecutionPolicy Bypass -File Scripts/open-insights.ps1 -Label <라벨>-rN`. 읽는 순서는 [insights-reading.md](Guides/insights-reading.md)에 있다. 측정 구간의 Timing 값(서버 프레임 시간 평균과 P99, `GameNetDriver` 아래 클래스별 프레임당 값)은 창 없이 `powershell -ExecutionPolicy Bypass -File Scripts/export-insights.ps1 -Label <라벨>-rN`으로 얻는다(결과는 `Saved/InsightsExport/<라벨>-rN/summary.txt`).
- 두 클라이언트 영상(1번 내려다보기, 2번 3인칭 이동): 이 PC의 150% 배율에서 두 창은 959,-47과 1919,-47(각 962×588)이라 `capture-video.ps1 -Region "959,0,1922,541" -RaiseSlots "1,2" -NoMouse`로 찍는다. `-Region`은 창을 맨 위로 올리지 않으므로 `-RaiseSlots`로 두 창(명령줄 `-LabSlot=1`, `-LabSlot=2`)을 찍는 동안 TOPMOST로 올린다(`visual2`, `visual3`에서는 같은 일을 `SetWindowPos`로 직접 했다. `-RaiseSlots`는 창이 없을 때 거부하는 것만 확인했고 실제 녹화에는 아직 쓰지 않았다). 8개 창 전체 화면은 `-RaiseSlots "0,1,2,3,4,5,6,7" -Region "0,0,3840,1126" -NoMouse -AllowMeasuring -Out <이름>.png`로 한 프레임을 찍는다(창은 4열 2줄, 각 962×588이고 영역이 창으로 모두 덮인다. `visual11`).
- 지난 태그의 빌드에서 녹화할 때: 태그를 체크아웃해 빌드한 뒤 `git restore --source=main --worktree Scripts/capture-video.ps1`로 지금의 녹화 스크립트만 꺼내 쓴다(태그 시점의 스크립트는 GIF 기본값이 다르다). 끝나면 `git restore Scripts/capture-video.ps1`로 되돌리고 main으로 돌아와 다시 빌드한다(`visual11`).
- NPC 하나의 움직임을 전후로 찍을 때: `run-scenario.ps1`에 `-ShowcaseNpc`를 더한다(`visualN` 라벨에서만 받는다. 0번 자리 앞 10m를 왕복하는 NPC 하나가 더 생긴다). 0번 창의 클라이언트 영역은 0,0 960×540이라 `capture-video.ps1 -Region "0,0,960,540" -Fps 60 -NoMouse -AllowMeasuring -Out <이름>.mp4`로 찍고, 자르기와 느린 재생은 ffmpeg로 따로 한다(`visual9`, `visual10`. 가공 값은 [candidates.md](../Posts/04-update-frequency/candidates.md) 6절).
- 수동 확인을 에이전트가 할 때: `run-manual.ps1`은 `Read-Host`로 기다리므로 `Start-Process powershell`로 새 창에 띄우고, 끝나면 `UnrealEditor`와 그 창을 종료한다(에이전트의 `Stop-Process`는 거부된다. troubleshooting.md). 컴퓨터 조작 권한은 `UnrealEditor.exe`의 전체 경로로 요청한다. 관찰자 창을 클릭하면 마우스가 카메라를 돌리므로 클릭한 뒤 커서를 옮겨 카메라를 맞춘다(커서의 창 안 x좌표에 따라 돌고, 조작 도구 좌표로 1px에 약 0.3°, 창 폭만큼만 돌릴 수 있다). 달리기(`shift+w`) 28초가 약 275m다(2026-10-02, 포스팅 3의 정확성 확인).
- 문서용 Insights 캡처: `powershell -ExecutionPolicy Bypass -File Scripts/capture-insights.ps1 -Label <라벨>-rN -Out <경로>.png`. 화면 복사 대신 창 내용만 찍는다.
- Insights 이미지에 번호 붙은 상자 그리기: `powershell -ExecutionPolicy Bypass -File Scripts/annotate-image.ps1 -In <원본>.png -Out <포스팅용 이름>.png -Boxes "x,y,w,h;x,y,w,h"`. 좌표는 원본 픽셀 기준, 상자는 최대 3개, 원본은 그대로 둔다.
- 빌드나 실행이 실패하면 [troubleshooting.md](Guides/troubleshooting.md)에서 증상을 찾는다.
- 수치 CSV 위치: `Saved/LabMetrics/summary.csv`(`config` 열이 있다. 2026-10-03 태스크 15 전의 행은 `summary-act1.csv`)
- 구성별 실행: 확정 명령에 기준선은 `-AlwaysRelevant -NoNodeDormancy -NpcUpdateFrequency 100`, Relevancy는 `-NoNodeDormancy -NpcUpdateFrequency 100`, Dormancy는 `-NpcUpdateFrequency 100`을 더한다(확정 규모에서 `toggle-baseline2`, `toggle-relevancy`, `toggle-dormancy`로 확인). 2막의 플레이어 배치는 밀집 `-PlayerSpacing 3`, 분산 `-PlayerSpacing 300`을 더한다. 상태 값은 `-StateInterval <초>`, 인벤토리는 `-InventoryItems <칸 수> -InventoryChurn <초>`, 건축물은 `-Buildings <무리 하나의 수> -BuildInterval <초>`, 플레이어 주변의 NPC는 `-NpcsNearPlayers <무리 하나의 수>`를 더한다. 보정의 출발값을 모두 켠 실행은 `-PlayerSpacing 3 -NpcsNearPlayers 50 -StateInterval 5 -InventoryItems 200 -InventoryChurn 4 -Buildings 500 -BuildInterval 1`이다([2막 설계](Planning/2026-10-03-act-2-design.md) 5.1, 확정값이 아니다). 더하지 않으면 세 기법이 모두 적용된 구성이다(`config` 열 `default`). 2막의 기법은 기본값이 끔이다: 자원 노드의 Net Update Frequency는 `-NodeUpdateFrequency 2`(포스팅 8). 간격 배치의 0번 자리는 클라이언트 수로 정해지므로(`GetSlotLocation`), 배치를 확인하는 작은 규모 실행은 `-Clients 8`로 한다(`tsmall-node-gather8-r1`)
- stat 타이머로 시간을 나누는 실행: `run-scenario.ps1`에 `-StatNamedEvents`를 더한다(서버에 `-statnamedevents`, 트레이스 필요). 이 실행의 수치는 다른 실행과 비교하지 않고 비율만 본다. `export-insights.ps1`이 서버 로그의 명령줄을 보고 `GameNetDriver`를 뿌리로 내보내고 `GameNetDriver`, `TickCompletionEvents` 아래 트리를 요약에 적는다. 이 트레이스에서는 `WorldTick`이 프레임을 감싸지 않고, 클래스 타이머는 `Replicate Actor Time` 아래에 있다(engine-notes.md 차절)
- 리플리케이션 시간으로 쓰는 Insights 타이머: `GameNetDriver`(프레임당 Incl = 선택 구간의 Incl ÷ `WorldTick`의 Count). 태스크 8.5에서 사용자가 확정했고 이후 바꾸지 않는다.

## 확정할 값

단기 설계 문서 8절의 출발값을 엔진 소스 확인(태스크 2.5)과 실측(태스크 8)으로 확정해 여기에 적는다. 측정 규약은 [measurement.md](Guides/measurement.md)에 있다.

| 항목 | 출발값 | 확정값 | 근거 |
| --- | --- | --- | --- |
| 클라이언트 수 | 8 | 8 | 측정값: `calib-a-r1` 실행 중 UE 프로세스 9개의 메모리 합계 27.5GB, 사용 가능 메모리 최저 14.9GB로 줄일 필요가 없었다(태스크 8.2) |
| 자원 노드 수 | 5,000 | 5,000(와 검증용 1개, 합 5,001) | 측정값: 포화가 없는 실행에서 `over_budget_frames`가 `frames`와 같아(`calib-b-r1` 100/100, `calib-f-r1` 356/356) 늘리지 않았다(태스크 8.4) |
| AI NPC 수 | 300 | 300 | 출발값 그대로. 네 조건을 모두 만족해 바꾸지 않았다(태스크 8.6) |
| 준비 구간 | 30초 | 30초 | 측정값: `calib-f-r1`에서 측정 시작 22초 전에 `open_actor_channels_per_conn`이 5,314에 도달해 더 늘지 않았다(태스크 8.3) |
| 측정 구간 | 60초 | 60초 | 출발값 그대로. Insights에서 두 북마크 사이가 60.018초(`calib-f-r1`) |
| 서버 코어 | 논리 프로세서 0\~7 | 논리 프로세서 2\~7(마스크 252) | [ADR-0009](Decisions/0009-server-cores-without-dpc-load.md). 측정값: `diag-d-r1`, `calib-f-r1` |
| 리플리케이션 시간 타이머 | 미정 | `GameNetDriver` | 측정값: `calib-f-r1` 측정 구간에서 Incl 57.45초, `WorldTick`의 96.01%. 프레임당 161.4ms가 CSV `netflush_avg_ms` 161.744와 0.2% 차이(태스크 8.5, 사용자 확정) |
| `NetServerMaxTickRate` 기본값 | 30 (기억값) | 30 | 엔진 소스: `Engine/Config/BaseEngine.ini:1867`. 실행 중 적용값도 30(30초에 898프레임, `smoke2-r1`) |
| `NetCullDistanceSquared` 기본값 | 225,000,000 (기억값) | 225,000,000 (150m) | 엔진 소스: `Engine/Source/Runtime/Engine/Private/Actor.cpp:312` |
| `NetUpdateFrequency` 기본값 | 100 (기억값) | 100 (`MinNetUpdateFrequency` 2) | 엔진 소스: `Actor.cpp:295-296` |
| 연결당 송신 한도(엔진 기본값) | 모름 | 100,000바이트/초 | 엔진 소스: `BaseEngine.ini:1839-1840, 1860-1861`. 올릴 때는 세 키를 함께 올린다(engine-notes.md 가절) |
| 연결당 송신 한도(이 프로젝트에서 고정한 값) | 기준선 실측 송신량을 30Hz로 환산한 값의 약 두 배(구현 계획 8.4a) | 350,000바이트/초 | 계산값: `calib-b-r1`의 `out_bytes_per_sec_per_conn` 9,591, 실제 틱 100 ÷ 60 = 1.67Hz, 환산 송신량 9,591 × 30 ÷ 1.67 = 172,638, 두 배 345,276을 올림. 바꾼 키: `Config/DefaultEngine.ini`의 `[/Script/Engine.Player] ConfiguredInternetSpeed`, `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate`, `MaxInternetClientRate`. 서버 로그 `net_speed=350000`과 `saturated_ratio` 0.000 확인(`calib-e-r1`) |
| 리플리케이션 시스템 | 레거시여야 함 | 레거시(소스 기준) | 엔진 소스: `IrisConfig.cpp:15-16`의 `net.Iris.UseIrisReplication` 기본값 0. 서버 로그 `using replication model Generic` 확인(`smoke2-r1`) |
| 2막: 플레이어 배치와 주변 NPC | 밀집 3m, 무리 하나 50개 | 밀집 3m(`-PlayerSpacing 3`), 주변 NPC 50개(`-NpcsNearPlayers 50`). 분산 300m는 비교 구성 | 사용자 확정(2026-10-04). 기준선 `calib2-d3-r1`: `over_budget_frames` 325 = `frames`, 측정 시작 21초 전에 채널 5,871, `saturated_ratio` 0.000, `GameNetDriver`가 `WorldTick`의 96.1%. Net Update Frequency가 `work_avg_ms`를 5.329 줄인다(`calib2-b-r1` 20.011, `calib2-a-r1` 14.682). 밀집과 분산은 구별된다(16.779, 27.347). 계산은 [2막 설계](Planning/2026-10-03-act-2-design.md) 5.1, 실행은 [Worklog/05-expanded-testbed.md](Worklog/05-expanded-testbed.md) |
| 2막: 건축물 | 무리 하나 500개, 짓는 간격 1초 | 500개, 1초(`-Buildings 500 -BuildInterval 1`) | 사용자 확정(2026-10-04). Relevancy만 적용한 구성이 틱 예산을 넘고(`calib2-c-r1` `work_avg_ms` 35.717) Dormancy가 15.706을 줄인다(`calib2-b-r1` 20.011). 기준선에서 `LabBuilding`은 프레임당 8.42ms(`calib2-d3-r1`) |
| 2막: 상태 값, 인벤토리 | 상태 값 간격 5초, 인벤토리 200칸에 4초 | 상태 값 프로퍼티 8개에 간격 5초(`-StateInterval 5`), 인벤토리 200칸에 앞 칸을 지우는 간격 4초(`-InventoryItems 200 -InventoryChurn 4`) | 사용자 확정(2026-10-04). `LabStateComponent` 프레임당 0.264ms(`calib2-a-r1`)는 변동 폭보다 작고 프로퍼티 64개에서도 0.327ms라(`calib2-e2-r1`) Push Model만 조건의 예외로 둔다. `LabInventoryComponent`는 `Actor` 비트의 29.4%(`layout-dense1-r1`) |

## 기준선 조건

1막은 단기 구현 계획 태스크 8.4\~8.6, 2막은 2막 구현 계획 태스크 21.1(`act2-baseline1`)에서 채웠다.

| 조건 | 결과 | 근거 |
| --- | --- | --- |
| 초기 전송 완료 | 예 | `calib-f-r1` 서버 로그: 측정 시작(13:57:10 UTC) 22초 전의 줄(13:56:48)에서 `open_actor_channels_per_conn`이 이미 5,314이고 더 늘지 않음 |
| 지속적인 예산 초과 | 예 | `calib-f-r1`: `over_budget_frames` 356 = `frames` 356. `work_avg_ms` 168.309는 틱 예산 33.3ms(1 ÷ 30Hz)의 5.0배 |
| 가장 큰 비용이 네트워크 | 예(사용자 판단, 2026-10-01) | `calib-f-r1`의 Insights 측정 구간: `WorldTick` 59.84초 중 `GameNetDriver` 57.45초(96.01%). [insights-walkthrough-calib-f.md](Guides/insights-walkthrough-calib-f.md) 6단계 |
| 송신 한도에 포화되지 않음(포화되면 한도를 올린다. 2026-10-01 결정) | 예(한도를 350,000으로 올린 뒤) | 엔진 기본 한도에서는 `saturated_ratio` 1.000(`calib-a-r1`). 350,000에서 0.000(`calib-f-r1`), 측정 구간에 `saturated_replications`의 앞 숫자가 211에서 늘지 않음 |
| 2막: 초기 전송 완료 | 예 | 서버 로그: `open_actor_channels_per_conn`이 측정 시작 19초, 16초, 20초 전(`r1`, `r2`, `r3`)에 5,871에 도달해 더 늘지 않음 |
| 2막: 지속적인 예산 초과 | 예 | 세 실행 모두 `over_budget_frames` = `frames`(279, 278, 272). `work_avg_ms` 중앙값 215.801은 틱 예산 33.3ms의 6.47배 |
| 2막: 가장 큰 비용이 네트워크 | 예(사용자 판단, 2026-10-04) | 세 실행의 Insights 측정 구간에서 `GameNetDriver`가 `WorldTick`의 95.74\~95.85%. CSV로는 206.944 ÷ 215.801 = 95.9%(중앙값). [candidates.md](../Posts/05-expanded-testbed/candidates.md) 2절 |
| 2막: 송신 한도에 포화되지 않음 | 예 | 세 실행 모두 `saturated_ratio` 0.000. 측정 구간에 `saturated_replications`의 앞 숫자가 259, 258, 261에서 늘지 않음. 30Hz 환산 송신량은 36,994 × 30 ÷ (278 ÷ 60) = 239,529바이트/초로 한도 350,000의 68%(`r2`) |

## 측정 결과

CSV 값을 CSV 열 이름 그대로 적는다. 구성마다 세 실행의 값을 실행 라벨과 함께 적고, 그 아래에 중앙값과 변동 폭(최댓값 - 최솟값)을 적는다. 포스팅과 README의 표에 쓰는 Insights 값은 여기가 아니라 각 포스팅에 적는다.

| 라벨 | frames | work_avg_ms | work_p99_ms | over_budget_frames | netflush_avg_ms | out_bytes_per_sec_per_conn | open_actor_channels_per_conn | saturated_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `act2-baseline1-r1` | 279 | 215.238 | 261.636 | 279 | 206.558 | 37065 | 5871 | 0.000 |
| `act2-baseline1-r2` | 278 | 215.801 | 270.781 | 278 | 206.944 | 36994 | 5871 | 0.000 |
| `act2-baseline1-r3` | 272 | 220.521 | 331.528 | 272 | 211.365 | 36324 | 5871 | 0.000 |
| **중앙값** | 278 | 215.801 | 270.781 | 278 | 206.944 | 36994 | 5871 | 0.000 |
| **변동 폭** | 7 | 5.283 | 69.892 | 7 | 4.807 | 741 | 0 | 0.000 |
| `act2-nodeuf-base1-r1` | 1789 | 17.418 | 26.867 | 1 | 13.141 | 16654 | 77 | 0.000 |
| `act2-nodeuf-base1-r2` | 1785 | 18.830 | 30.673 | 9 | 14.313 | 16603 | 77 | 0.000 |
| `act2-nodeuf-base1-r3` | 1789 | 18.060 | 26.008 | 1 | 13.605 | 16611 | 77 | 0.000 |
| **중앙값** | 1789 | 18.060 | 26.867 | 1 | 13.605 | 16611 | 77 | 0.000 |
| **변동 폭** | 4 | 1.412 | 4.665 | 8 | 1.172 | 51 | 0 | 0.000 |
| `act2-nodeuf1-r1` | 1794 | 8.557 | 13.373 | 0 | 4.170 | 16570 | 77 | 0.000 |
| `act2-nodeuf1-r2` | 1796 | 8.780 | 13.807 | 0 | 4.296 | 16659 | 77 | 0.000 |
| `act2-nodeuf1-r3` | 1795 | 8.615 | 13.477 | 0 | 4.177 | 16569 | 77 | 0.000 |
| **중앙값** | 1795 | 8.615 | 13.477 | 0 | 4.177 | 16570 | 77 | 0.000 |
| **변동 폭** | 2 | 0.223 | 0.434 | 0 | 0.126 | 90 | 0 | 0.000 |
| **`act2-nodeuf-base1` 대비** | +6 | -9.445(-52.3%) | -13.390(-49.8%) | -1 | -9.428(-69.3%) | -41(-0.2%) | 0 | 0 |

1막 네 묶음(`baseline3`\~`update-frequency3`)의 행과 경위는 [Worklog/04-update-frequency.md](Worklog/04-update-frequency.md)의 "태스크 25 준비"로 옮겼다. `act2-baseline1`의 실행 경위는 [포스팅 5 측정 기록](../Posts/05-expanded-testbed/measurements.md) 1절에 있다. `act2-nodeuf-base1`(포스팅 6의 최종 구성)과 `act2-nodeuf1`(`-NodeUpdateFrequency 2`)은 2026-10-06에 연달아 쟀고, 타이머를 더 켠 두 묶음과 Insights 값은 [포스팅 8 관찰 자료](../Posts/08-node-update-frequency/candidates.md) 6절부터에 있다.

## 막힌 것

- 없음.

## 사용자에게 요청한 일

- **다시 쓴 포스팅 네 편 확인.** 위 "다음 할 일" 1번.
- **[ADR-0018](Decisions/0018-post-length-in-characters.md) 승인.** ADR-0017의 "6,000\~8,000자"는 UTF-8 바이트였다. 글자 수 2,700\~3,700자로 바꾼다. 승인되면 posting.md와 AGENTS.md의 숫자를 고치고 0017을 "대체됨"으로 바꾼다.