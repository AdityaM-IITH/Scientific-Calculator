#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef enum
{
  TOKEN_NUMBER,
  TOKEN_OP,
  TOKEN_FUNC,
  TOKEN_BRAC
} TokenType;
typedef enum
{
  OP_PLUS,
  OP_MINUS,
  OP_MUL,
  OP_DIV,
  OP_POW,
  OP_NEG
} OperatorType;
typedef enum
{
  FUNC_SIN,
  FUNC_COS,
  FUNC_TAN,
  FUNC_ARCSIN,
  FUNC_ARCCOS,
  FUNC_ARCTAN,
  FUNC_LN
} FunctionType;
typedef enum
{
  BRAC_OPEN,
  BRAC_CLOSE
} BracketType;

typedef struct
{
  TokenType type;
  union
  {
    double number;
    OperatorType op;
    FunctionType func;
    BracketType brac;
  };
} Token;

typedef struct
{
  double data[100];
  int top;
} ValStack;

int g_error = 0;

double my_fabs(double x) { return x < 0 ? -x : x; }

double my_sqrt(double x)
{
  if (x < 0)
    return 0.0 / 0.0;
  double r = x;
  for (int i = 0; i < 30; i++)
    r = 0.5 * (r + x / r);
  return r;
}

double my_pow(double a, double b)
{
  int exp = (int)b;
  double result = 1.0;
  if (exp < 0)
  {
    exp = -exp;
    for (int i = 0; i < exp; i++)
      result *= a;
    return 1.0 / result;
  }
  for (int i = 0; i < exp; i++)
    result *= a;
  return result;
}

int my_isfinite(double x)
{
  return (x == x) && (x != 1.0 / 0.0) && (x != -1.0 / 0.0);
}

double my_nan() { return 0.0 / 0.0; }

#define MY_PI 3.14159265358979323846

void vs_init(ValStack *s) { s->top = -1; }
void vs_push(ValStack *s, double v)
{
  if (s->top < 99)
    s->data[++s->top] = v;
  else
    g_error = 1;
}
double vs_pop(ValStack *s)
{
  if (s->top < 0)
  {
    g_error = 1;
    return 0;
  }
  return s->data[s->top--];
}

int tokenize(char *expr, Token *tokens)
{
  int curr = 0, cnt = 0;
  while (expr[curr])
  {
    if (isspace((unsigned char)expr[curr]))
    {
      curr++;
      continue;
    }

    if (isdigit((unsigned char)expr[curr]) || expr[curr] == '.')
    {
      double value = 0, frac = 0.1;
      int seen_dot = 0;
      while (isdigit((unsigned char)expr[curr]) || expr[curr] == '.')
      {
        if (expr[curr] == '.')
        {
          if (seen_dot)
          {
            g_error = 1;
            return -1;
          }
          seen_dot = 1;
        }
        else
        {
          int d = expr[curr] - '0';
          if (!seen_dot)
            value = value * 10 + d;
          else
          {
            value += d * frac;
            frac *= 0.1;
          }
        }
        curr++;
      }
      Token t;
      t.type = TOKEN_NUMBER;
      t.number = value;
      tokens[cnt++] = t;
      continue;
    }

    if (isalpha((unsigned char)expr[curr]))
    {
      int start = curr;
      while (isalpha((unsigned char)expr[curr]))
        curr++;
      int len = curr - start;
      Token t;
      t.type = TOKEN_FUNC;

      if (len == 3 && strncmp(&expr[start], "sin", 3) == 0)
        t.func = FUNC_SIN;
      else if (len == 3 && strncmp(&expr[start], "cos", 3) == 0)
        t.func = FUNC_COS;
      else if (len == 3 && strncmp(&expr[start], "tan", 3) == 0)
        t.func = FUNC_TAN;
      else if (len == 6 && strncmp(&expr[start], "arcsin", 6) == 0)
        t.func = FUNC_ARCSIN;
      else if (len == 6 && strncmp(&expr[start], "arccos", 6) == 0)
        t.func = FUNC_ARCCOS;
      else if (len == 6 && strncmp(&expr[start], "arctan", 6) == 0)
        t.func = FUNC_ARCTAN;
      else if (len == 2 && strncmp(&expr[start], "ln", 2) == 0)
        t.func = FUNC_LN;
      else
      {
        g_error = 1;
        return -1;
      }

      tokens[cnt++] = t;
      continue;
    }

    if (strchr("+-*/^", expr[curr]))
    {
      Token t;
      t.type = TOKEN_OP;
      if (expr[curr] == '-')
      {
        if (cnt == 0 || tokens[cnt - 1].type == TOKEN_OP ||
            (tokens[cnt - 1].type == TOKEN_BRAC && tokens[cnt - 1].brac == BRAC_OPEN))
          t.op = OP_NEG;
        else
          t.op = OP_MINUS;
      }
      else if (expr[curr] == '+')
        t.op = OP_PLUS;
      else if (expr[curr] == '*')
        t.op = OP_MUL;
      else if (expr[curr] == '/')
        t.op = OP_DIV;
      else
        t.op = OP_POW;

      tokens[cnt++] = t;
      curr++;
      continue;
    }

    if (expr[curr] == '(' || expr[curr] == ')')
    {
      Token t;
      t.type = TOKEN_BRAC;
      t.brac = (expr[curr] == '(') ? BRAC_OPEN : BRAC_CLOSE;
      tokens[cnt++] = t;
      curr++;
      continue;
    }

    g_error = 1;
    return -1;
  }
  return cnt;
}

int precedence(OperatorType op)
{
  if (op == OP_NEG)
    return 4;
  if (op == OP_POW)
    return 3;
  if (op == OP_MUL || op == OP_DIV)
    return 2;
  if (op == OP_PLUS || op == OP_MINUS)
    return 1;
  return 0;
}
int is_right_associative(OperatorType op) { return (op == OP_POW || op == OP_NEG); }

int shunting_yard(Token *infix, int n, Token *postfix)
{
  Token stack[100];
  int top = -1, out = 0;
  for (int i = 0; i < n; i++)
  {
    Token curr = infix[i];
    if (curr.type == TOKEN_NUMBER)
      postfix[out++] = curr;
    else if (curr.type == TOKEN_FUNC)
      stack[++top] = curr;
    else if (curr.type == TOKEN_OP)
    {
      while (top >= 0 && stack[top].type == TOKEN_OP &&
             (precedence(stack[top].op) > precedence(curr.op) ||
              (precedence(stack[top].op) == precedence(curr.op) && !is_right_associative(curr.op))))
        postfix[out++] = stack[top--];
      stack[++top] = curr;
    }
    else if (curr.type == TOKEN_BRAC && curr.brac == BRAC_OPEN)
      stack[++top] = curr;
    else if (curr.type == TOKEN_BRAC && curr.brac == BRAC_CLOSE)
    {
      while (top >= 0 && !(stack[top].type == TOKEN_BRAC && stack[top].brac == BRAC_OPEN))
        postfix[out++] = stack[top--];
      if (top < 0)
      {
        g_error = 1;
        return -1;
      }
      top--;
      if (top >= 0 && stack[top].type == TOKEN_FUNC)
        postfix[out++] = stack[top--];
    }
  }
  while (top >= 0)
  {
    if (stack[top].type == TOKEN_BRAC)
    {
      g_error = 1;
      return -1;
    }
    postfix[out++] = stack[top--];
  }
  return out;
}

double rk4(double (*f)(double, double), double x0, double y0, double x1, int steps)
{
  double h = (x1 - x0) / steps, x = x0, y = y0;
  for (int i = 0; i < steps; i++)
  {
    double k1 = h * f(x, y);
    double k2 = h * f(x + h / 2, y + k1 / 2);
    double k3 = h * f(x + h / 2, y + k2 / 2);
    double k4 = h * f(x + h, y + k3);
    y += (k1 + 2 * k2 + 2 * k3 + k4) / 6;
    x += h;
  }
  return y;
}

void rk4_sincos(double x1, int steps, double *sinx, double *cosx)
{
  double h = x1 / steps, y = 0, z = 1;
  for (int i = 0; i < steps; i++)
  {
    double k1y = h * z, k1z = h * (-y);
    double k2y = h * (z + k1z / 2), k2z = h * (-(y + k1y / 2));
    double k3y = h * (z + k2z / 2), k3z = h * (-(y + k2y / 2));
    double k4y = h * (z + k3z), k4z = h * (-(y + k3y));
    y += (k1y + 2 * k2y + 2 * k3y + k4y) / 6;
    z += (k1z + 2 * k2z + 2 * k3z + k4z) / 6;
  }
  *sinx = y;
  *cosx = z;
}

double d_ln(double x, double y)
{
  if (x <= 0)
    return my_nan();
  return 1.0 / x;
}
double d_atan(double x, double y) { return 1.0 / (1.0 + x * x); }
double d_asin(double x, double y)
{
  if (my_fabs(x) > 1)
    return my_nan();
  return 1.0 / my_sqrt(1.0 - x * x);
}

double eval_function(FunctionType f, double x)
{
  const int STEPS = 1000;
  switch (f)
  {
  case FUNC_SIN:
  {
    double s, c;
    rk4_sincos(x, STEPS, &s, &c);
    return s;
  }
  case FUNC_COS:
  {
    double s, c;
    rk4_sincos(x, STEPS, &s, &c);
    return c;
  }
  case FUNC_TAN:
  {
    double s, c;
    rk4_sincos(x, STEPS, &s, &c);
    if (my_fabs(c) < 1e-15)
      return my_nan();
    return s / c;
  }
  case FUNC_LN:
    if (x <= 0)
      return my_nan();
    return rk4(d_ln, 1.0, 0.0, x, STEPS);
  case FUNC_ARCTAN:
    return rk4(d_atan, 0.0, 0.0, x, STEPS);
  case FUNC_ARCSIN:
    if (x < -1 || x > 1)
      return my_nan();
    if (my_fabs(x - 1.0) < 1e-9)
      return MY_PI / 2;
    if (my_fabs(x + 1.0) < 1e-9)
      return -MY_PI / 2;
    return rk4(d_asin, 0.0, 0.0, x, STEPS);
  case FUNC_ARCCOS:
    return MY_PI / 2 - eval_function(FUNC_ARCSIN, x);
  default:
    return my_nan();
  }
}

double eval_operator(OperatorType op, double a, double b)
{
  switch (op)
  {
  case OP_PLUS:
    return a + b;
  case OP_MINUS:
    return a - b;
  case OP_MUL:
    return a * b;
  case OP_DIV:
    if (my_fabs(b) < 1e-15)
      return my_nan();
    return a / b;
  case OP_POW:
    return my_pow(a, b);
  default:
    return my_nan();
  }
}

double eval_postfix(const Token *postfix, int count)
{
  ValStack stack;
  vs_init(&stack);
  g_error = 0;
  for (int i = 0; i < count; i++)
  {
    Token t = postfix[i];
    if (t.type == TOKEN_NUMBER)
      vs_push(&stack, t.number);
    else if (t.type == TOKEN_OP)
    {
      if (t.op == OP_NEG)
      {
        double x = vs_pop(&stack);
        vs_push(&stack, -x);
      }
      else
      {
        double b = vs_pop(&stack), a = vs_pop(&stack);
        double r = eval_operator(t.op, a, b);
        if (!my_isfinite(r))
          return my_nan();
        vs_push(&stack, r);
      }
    }
    else if (t.type == TOKEN_FUNC)
    {
      double x = vs_pop(&stack);
      double r = eval_function(t.func, x);
      if (!my_isfinite(r))
        return my_nan();
      vs_push(&stack, r);
    }
    if (g_error)
      return my_nan();
  }
  if (stack.top != 0)
    return my_nan();
  return vs_pop(&stack);
}

int main()
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(8080);

  bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
  listen(server_fd, 5);

  while (1)
  {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
      continue;

    char buf[1024];
    int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
    {
      close(client_fd);
      continue;
    }
    buf[n] = '\0';

    Token tokens[100], postfix[100];
    g_error = 0;

    int len = tokenize(buf, tokens);
    int post_len = (g_error) ? -1 : shunting_yard(tokens, len, postfix);

    if (g_error || len <= 0 || post_len <= 0)
    {
      send(client_fd, "ERR\n", 4, 0);
    }
    else
    {
      double result = eval_postfix(postfix, post_len);
      if (!my_isfinite(result))
      {
        send(client_fd, "ERR\n", 4, 0);
      }
      else
      {
        char out[128];
        snprintf(out, sizeof(out), "%.10f\n", result);
        send(client_fd, out, strlen(out), 0);
      }
    }

    close(client_fd);
  }
  return 0;
}