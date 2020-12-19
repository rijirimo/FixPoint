#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128
#define LINE_SIZE 1e3

int line_count(char *fname) {
  int line = 0;
  char *buf;
  FILE *fp;
  if ((fp = fopen(fname, "r")) == NULL) {
    printf("File open error:%s\n", fname);
    return -1;
  }

  buf = malloc(sizeof(char *) * BUF_SIZE);

  while (fgets(buf, BUF_SIZE, fp) != NULL) {
    line++;
  }

  free(buf);
  return line;
}

char **mal(int line) {
  int i;
  char **tmp;
  tmp = malloc(sizeof(char *) * line);
  for (i = 0; i < LINE_SIZE; i++) {
    tmp[i] = malloc(sizeof(char *) * BUF_SIZE);
  }

  return tmp;
}

void cpytime(char **time, int n1, int n2, int *h, int *m, int *s) {
  char buf[2][5];
  int i;
  for (i = 0; i < 3; i++) {
    if (i == 0) {
      strncpy(buf[0], time[n1] + 8, 2);
      strncpy(buf[1], time[n2] + 8, 2);
      // printf("time[%d] = %s\n", n1, time[n1]);
      // printf("time[%d] = %s\n", n2, time[n2]);

      buf[0][2] = '\0';
      buf[1][2] = '\0';
      // printf("buf[0] = %s\n", buf[0]);
      // printf("buf[1] = %s\n", buf[1]);

      h[0] = atoi(buf[0]);
      h[1] = atoi(buf[1]);
      // printf("h[0] = %d\n", h[0]);
      // printf("h[1] = %d\n", h[1]);
    } else if (i == 1) {
      strncpy(buf[0], time[n1] + 10, 2);
      strncpy(buf[1], time[n2] + 10, 2);

      buf[0][2] = '\0';
      buf[1][2] = '\0';

      m[0] = atoi(buf[0]);
      m[1] = atoi(buf[1]);
      // printf("m[0] = %d\n", m[0]);
      // printf("m[1] = %d\n", m[1]);
    } else if (i == 2) {
      strncpy(buf[0], time[n1] + 12, 2);
      strncpy(buf[1], time[n2] + 12, 2);

      buf[0][2] = '\0';
      buf[1][2] = '\0';

      s[0] = atoi(buf[0]);
      s[1] = atoi(buf[1]);
    }
  }
}

void calc_fail(char **time, char **IP, char **ping, int line, int *flag,
               int N) {
  int t1, t2, i, j, n1, n2, period;
  int h[2], m[2], s[2];
  int hour, min, sec, ms;
  char buf[3];
  int count = 0;
  char start[128];
  for (i = 0; i < line; i++) {
    if (flag[i] == 0) {
      if (ping[i][0] == '-') {
        n1 = i;
        count++;
        // printf("故障状態のサーバアドレス：%s\n", IP[i]);
        // printf("故障開始日時：%s\n", time[i]);

        for (j = i + 1; j < line; j++) {
          if ((strcmp(IP[i], IP[j])) == 0 && ping[j][0] == '-') {
            flag[j] = 1;
            count++;
            printf("count = %d\n", count);
          }
          if ((strcmp(IP[i], IP[j])) == 0 && ping[j][0] != '-') {
            if (count >= N) {
              n2 = j;
              printf("故障状態のサーバアドレス：%s\n", IP[i]);
              printf("故障開始日時：%s\n", time[i]);
              printf("故障復旧日時：%s\n", time[j]);
              count = 0;
              break;
            }
          }
          if (j == line - 1) {
            n2 = j;
            if (count < N) {
              printf("故障したサーバはありませんでした．\n");
            } else {
              printf("故障は復旧してません．\n");
            }
          }
        }

        if (n2 != line - 1) {
          cpytime(time, n1, n2, h, m, s);
          ms = atoi(ping[n2]);
          t1 = m[0] * 60 + s[0];
          t2 = m[1] * 60 + s[1];
          period = t2 - t1;

          printf("故障期間：");
          if (period / 60 < 10) {
            printf("0%d分", period / 60);
          } else if (period / 60 >= 10) {
            printf("%d分", period / 60);
          }
          if (period % 60 < 10) {
            printf("0%d秒", period % 60);
          } else if (period % 60 >= 10) {
            printf("%d秒", period % 60);
          }
          if (ms > 100) {
            printf("%dミリ秒\n", ms);
          } else if (ms > 10) {
            printf("0%dミリ秒\n", ms);
          } else {
            printf("00%dミリ秒\n", ms);
          }
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  char **time, **IP, **ping;
  int *flag;
  char data[BUF_SIZE], fname[BUF_SIZE];
  int t1, t2, line, i, j, N;
  int failure_period;
  int count = 0;

  sprintf(fname, "%s", argv[1]);
  printf("%s\n", fname);
  line = line_count(fname);
  if (line == -1) {
    return 0;
  }
  printf("line = %d\n", line);

  flag = malloc(sizeof(int *) * line);
  for (i = 0; i < line; i++) flag[i] = 0;

  N = atoi(argv[2]);
  printf("N = %d\n", N);

  time = mal(line);
  IP = mal(line);
  ping = mal(line);

  FILE *fp;
  if ((fp = fopen(fname, "r")) == NULL) {
    printf("File open error : log.txt\n");
    return -1;
  }

  while ((fgets(data, BUF_SIZE, fp)) != NULL) {
    sscanf(data, "%[^,],%[^,],%s", time[count], IP[count], ping[count]);
    // printf("%s,%s,%s\n", time[count], IP[count], ping[count]);
    count++;
  }

  calc_fail(time, IP, ping, line, flag, N);

  fclose(fp);
  free(time);
  free(IP);
  free(ping);
  free(flag);
  return 0;
}