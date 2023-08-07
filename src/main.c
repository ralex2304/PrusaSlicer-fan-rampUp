#include <stdio.h>
#include "gcode.h"
#include "files.h"
#include "cvector.h"

cvector v_lines;
cvector v_fan_lines;
void process(cvector* lines, double delay, cvector* fan_lines);
int fan_stack_proc(cvector* lines, cvector* fan_lines, GcodeLine* line, const unsigned long target_index, const double current_fan);

int main(int argc, char* argv[]){
  cvector_init(&v_lines, sizeof(GcodeLine));
  cvector_init(&v_fan_lines, sizeof(unsigned long));
  if(argc != 3)
    throw_error(ERR_CONSOLE_ARGUMENTS);
  file_read_lines(argv[2], &v_lines);
  process(&v_lines, strtod(argv[1], NULL), &v_fan_lines);
  file_write_lines(argv[2], &v_lines);
  return 0;
}

void process(cvector* lines, double delay, cvector* fan_lines){
  if(delay <= 0.0)
    throw_error(ERR_CONSOLE_ARGUMENTS);
  static double current_fan = 0;
  GcodeLine* line;
  for(unsigned long i = 0; i < lines->size; i++){
    line = cvector_get(lines, i);
    if(startswith(line->str, L"M107")){
      cvector_push(fan_lines, &i);
      current_fan = 0.0;
    } else if(startswith(line->str, L"M106")){
      if(get_param(line->str, L"S") <= current_fan){
        line->pos.fan_prev = current_fan;
        current_fan = get_param(line->str, L"S");
        cvector_push(fan_lines, &i);
      } else{
        line->pos.fan_prev = current_fan;
        current_fan = get_param(line->str, L"S");
        double target_time = line->time - delay;
        unsigned long target_index = find_line_by_time(lines, target_time);
        if(fan_stack_proc(lines, fan_lines, line, target_index, current_fan) == 0){
          continue;
        }
        GcodeLine* bef_line;
        if(target_index == 0)
          bef_line = cvector_get(lines, 0);
        else
          bef_line = cvector_get(lines, target_index-1);
        GcodeLine* line1 = NULL;
        GcodeLine* line2 = NULL;
        if(target_time == bef_line->time || target_index == 0 || !gcode_split_line(cvector_get(lines, target_index), bef_line, target_time-bef_line->time, &line1, &line2)){
          GcodeLine new_line = *bef_line;
          new_line.str = line->str;
          cvector_insert(lines, target_index, &new_line);
          i++;
          cvector_push(fan_lines, &target_index);
        } else{
          GcodeLine new_line = *line1;
          new_line.str = line->str;
          cvector_set(lines, target_index, line2);
          cvector_insert(lines, target_index, &new_line);
          cvector_insert(lines, target_index, line1);
          free(line1);
          free(line2);
          i += 2;
          target_index++;
          cvector_push(fan_lines, &target_index);
        }
        cvector_delete(lines, i);
        i--;
      }
    }
  }
  
  return;
}

int fan_stack_proc(cvector* lines, cvector* fan_lines, GcodeLine* line, const unsigned long target_index, const double current_fan){
  while(fan_lines->size && *((unsigned long*)cvector_get(fan_lines, fan_lines->size-1)) >= target_index){
    GcodeLine* fan_line = cvector_get(lines, *((unsigned long*)cvector_get(fan_lines, fan_lines->size-1)));
    if(startswith(fan_line->str, L"M107")){
      fan_line->str = realloc(fan_line->str, 1 * sizeof(wchar_t));
      *fan_line->str = '\0';
      cvector_pop(fan_lines);
    } else{
      if(fan_line->pos.fan_prev >= current_fan){
        fan_line->str = realloc(fan_line->str, (wcslen(line->str)+1) * sizeof(wchar_t));
        wcscpy(fan_line->str, line->str);
        free(line->str);
        return 0;
      } else{
        fan_line->str = realloc(fan_line->str, 1 * sizeof(wchar_t));
        *fan_line->str = '\0';
        cvector_pop(fan_lines);
      }
    }
  }
  return 1;
}
