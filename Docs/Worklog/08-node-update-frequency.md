# 작업 기록: 포스팅 8 자원 노드의 Net Update Frequency 낮추기 (태스크 25)

STATUS.md에서 옮긴 작업 기록이다. 옮길 때의 문장을 그대로 두고 상대 링크만 이 폴더 기준으로 바꿨다. 지난 항목은 고치지 않는다.

## 2026-10-06 태스크 25: 자원 노드의 Net Update Frequency 낮추기 (`act2-nodeuf-base1`, `act2-nodeuf1`, `tsmall-nodeuf-off1`, `tsmall-nodeuf-on1`)

- **태스크 25(포스팅 8, `post-08-node-update-frequency`)가 끝났다(2026-10-06).** 자원 노드의 Net Update Frequency를 2로 낮추자 `work_avg_ms` 중앙값이 18.060 → 8.615, 프레임당 Consider List가 3,887 → 413이었다(`act2-nodeuf-base1`, `act2-nodeuf1`). 활성 목록은 5,272 그대로라 `Consider Actors Time` 0.959ms가 남았고, 이제 `GameNetDriver`에서 가장 큰 것은 `Process Prioritized Actors Time`(45.8%)이다([측정 기록](../../Posts/08-node-update-frequency/measurements.md)). 아래는 진행 중의 기록이다. 사용자가 [후보](../../Posts/08-node-update-frequency/candidates.md) A를 골랐다. 구현과 작은 규모 확인이 끝났고, 빌드된 바이너리는 그 커밋의 소스다. 인자는 "명령"에 있다. 낮춘 구성에서는 채집과 되살아남이 `ForceNetUpdate()`로 다음 고려 시각을 당긴다(`LabResourceNode.cpp`의 `WakeForChange`). 새 서버 로그 `lab_consider_list avg_per_frame=`(측정 구간의 프레임당 Consider List 길이, 엔진 지표 `NumConsideredActors`)가 작은 규모에서 85.1 → 15.2였다(`tsmall-nodeuf-off1-r1`, `tsmall-nodeuf-on1-r1`, 클라이언트 2, 자원 노드 100). 낮춘 구성에서도 검증용 자원 노드가 고갈되어 화면에서 사라졌다.
- **[사람] 포스팅 8과 태그 푸시.** 초안을 사용자가 승인했고(2026-10-06) 태그 `post-08-node-update-frequency`를 붙였다.
