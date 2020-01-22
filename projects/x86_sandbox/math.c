typedef struct {
  int last_result;
} math__vars_s;

static math__vars_s math__vars;

int math__add(int x0, int x1) {
  int ret = x0 + x1;
  math__vars.last_result = ret;
  return ret;
}

static float math__private_get_last_result(void) { return math__vars.last_result; }
