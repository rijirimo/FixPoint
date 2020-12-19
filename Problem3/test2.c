#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128
#define LINE_SIZE 1e3

// ログファイルの行数を数える関数
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

// サーバ数を数える関数
int server_count(char **IP, char **tmp_IP, int line) {
  int i, j, IP_num;
  for (i = 0; i < line; i++) {
    if (i == 0) {
      strcpy(tmp_IP[i], IP[i]);
      IP_num = 1;
    }
    for (j = 0; j < IP_num; j++) {
      // printf("%d, %d\n", j, count);
      if ((strcmp(tmp_IP[j], IP[i])) == 0) break;
      if (j == IP_num - 1) {
        strcpy(tmp_IP[IP_num], IP[i]);
        IP_num++;
      }
    }
  }
  return IP_num;
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

double **mal_double(int n1, int n2) {
  int i;
  double **tmp;
  tmp = malloc(sizeof(double *) * n1);
  for (i = 0; i < LINE_SIZE; i++) {
    tmp[i] = malloc(sizeof(double *) * n2);
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
  int tmp1, tmp2, i, j, n1, n2;
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
            printf("故障は復旧してません．\n");
          }
        }

        if (n2 != line - 1) {
          cpytime(time, n1, n2, h, m, s);
          hour = h[1] - h[0];
          min = m[1] - m[0];
          sec = s[1] - s[0];
          ms = atoi(ping[i]);

          printf("故障期間：");
          if (hour < 10) {
            printf("0%d時間", hour);
          } else if (hour >= 10) {
            printf("%d時間", hour);
          }
          if (min < 10) {
            printf("0%d分", min);
          } else if (min >= 10) {
            printf("%d分", min);
          }
          if (sec < 10) {
            printf("0%d秒", sec);
          } else if (sec >= 10) {
            printf("%d秒", sec);
          }
          if (sec > 100) {
            printf("%dミリ秒\n", ms);
          } else if (sec > 10) {
            printf("0%dミリ秒\n", ms);
          } else {
            printf("00%dミリ秒\n", ms);
          }
        }
      }
    }
  }
}

void ave_ping(char **time, char **IP, char **ping, char **tmp_IP, int line,
              int IP_num, int m, int t) {
  int i, j, k, num;
  int *count;
  double *ave;
  double **sum;
  char tmp[BUF_SIZE];
  int ho[2], mi[2], se[2];
  int hour = 0, min = 0, sec = 0;
  int t1, t2, period;

  count = malloc(sizeof(int *) * IP_num);
  ave = malloc(sizeof(double *) * IP_num);
  sum = mal_double(IP_num, 2 * m);

  for (i = 0; i < IP_num; i++) {
    count[i] = 0;
    ave[i] = 0;

    for (j = 0; j < 2 * m; j++) {
      sum[i][j] = 0;
    }
  }

  for (i = 0; i < line; i++) {
    num = 0;
    for (j = 0; j < IP_num; j++) {
      if ((strcmp(IP[i], tmp_IP[j])) == 0) {
        if (ping[i][0] == '-') {
          printf("Server %s is time out!\n", IP[i]);
        }
        count[j]++;
        sum[j][count[j] % m] = atof(ping[i]);

        for (k = 0; k < m; k++) {
          ave[j] += sum[j][k] / m;
        }
        if (ave[j] > t) {
          printf("過負荷サーバ：%s\n", IP[i]);

          for (k = i - 1; k >= 0; k--) {
            if ((strcmp(IP[i], IP[k])) == 0) {
              num++;
            }
          }

          cpytime(time, i - num, i, ho, mi, se);
          t1 = mi[0] * 60 + se[0];
          t2 = mi[1] * 60 + se[1];
          period = t2 - t1;

          printf("期間：");
          if (period / 60 < 10) {
            printf("0%d分", period / 60);
          } else if (period / 60 >= 10) {
            printf("%d分", period / 60);
          }
          if (period % 60 < 10) {
            printf("0%d秒\n", period % 60);
          } else if (period % 60 >= 10) {
            printf("%d秒\n", period % 60);
          }
        }
        ave[j] = 0;
      }
    }
  }

  for (i = 0; i < IP_num; i++) {
    free(sum[i]);
  }

  free(count);
  free(ave);
}

int main(int argc, char *argv[]) {
  char **time, **IP, **ping, **tmp_IP;
  int *flag;
  char data[BUF_SIZE], fname[BUF_SIZE];
  int t1, t2, line, i, j, N, m, t, IP_num;
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
  m = atoi(argv[3]);
  t = atoi(argv[4]);
  printf("N = %d, m = %d, t = %d\n", N, m, t);

  time = mal(line);
  IP = mal(line);
  ping = mal(line);
  tmp_IP = mal(line);

  FILE *fp;
  if ((fp = fopen(fname, "r")) == NULL) {
    printf("File open error : log.txt\n");
    return -1;
  }

  while ((fgets(data, BUF_SIZE, fp)) != NULL) {
    sscanf(data, "%[^,],%[^,],%s", time[count], IP[count], ping[count]);
    count++;
  }

  // サーバ数を算出
  IP_num = server_count(IP, tmp_IP, line);

  // calc_fail(time, IP, ping, line, flag, N);

  ave_ping(time, IP, ping, tmp_IP, line, IP_num, m, t);

  fclose(fp);

  for (i = 0; i < line; i++) {
    free(time[i]);
    free(IP[i]);
    free(ping[i]);
    free(tmp_IP[i]);
  }
  free(flag);
  return 0;
}