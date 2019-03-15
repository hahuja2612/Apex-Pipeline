/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"


int MUL_INDEX=0;
int STOP_INDEX=0;
int INSTRUCT_INDEX=0;
int HALT_INDEX=0;
int RS1_INDEX=0;
int RS2_INDEX=0;

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1


/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;

    //z_flag
    cpu->z_flag[0]=0;
    
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

//  if (ENABLE_DEBUG_MESSAGES) {
//    fprintf(stderr,
//            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
//            cpu->code_memory_size);
//    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
//    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");
//
//    for (int i = 0; i < cpu->code_memory_size; ++i) {
//
//
//
//      printf("%-9s %-9d %-9d %-9d %-9d\n",
//             cpu->code_memory[i].opcode,
//             cpu->code_memory[i].rd,
//             cpu->code_memory[i].rs1,
//             cpu->code_memory[i].rs2,
//             cpu->code_memory[i].imm);
//    }
//  }

    
  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction_WB(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d MEM(%d)=%d", stage->opcode, stage->rs1, stage->rs2, stage->imm, stage->mem_address, stage->rs1_value);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d R%d(%d)", stage->opcode, stage->rd, stage->imm, stage->rd, stage->buffer);
  }
    
    if (strcmp(stage->opcode, "NOP") == 0) {
        printf("%s", stage->opcode);
    }
    
    if (strcmp(stage->opcode, "LOAD") == 0) {
        printf("%s,R%d,R%d,#%d R(%d)=%d", stage->opcode, stage->rd, stage->rs1, stage->imm, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "ADD") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "SUB") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "AND") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "MUL") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "BZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "BNZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "JUMP") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "HALT") == 0) {
        printf(
               "%s", stage->opcode);
    }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content_WB(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction_WB(stage);
  printf("\n");
}

static void
print_instruction(CPU_Stage* stage)
{
    if (strcmp(stage->opcode, "STORE") == 0) {
        printf(
               "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
    }
    
    if (strcmp(stage->opcode, "MOVC") == 0) {
        printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
    }
    
    
    if (strcmp(stage->opcode, "NOP") == 0) {
        printf("%s", stage->opcode);
    }
    
    if (strcmp(stage->opcode, "LOAD") == 0) {
        printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "ADD") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs1);
    }
    
    if (strcmp(stage->opcode, "SUB") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "AND") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "MUL") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "BZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "BNZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "JUMP") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "HALT") == 0) {
        printf(
               "%s", stage->opcode);
    }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
    CPU_Stage* stage_MEM = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

      
      
    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
      
      
      INSTRUCT_INDEX++;



    /* Update PC for next instruction */
    cpu->pc += 4;
      
      if (strcmp(stage->opcode, "") == 0) {
          strcpy(stage->opcode, "NOP");
          stage->rs2 = -1;
          stage->rs1 = -1;
      }

      if (cpu->pc >= (4000+ 4*(cpu->code_memory_size+1)) ){
          strcpy(stage->opcode, "NOP");
          stage->rs2 = -1;
          stage->rs1 = -1;
      }

      
//      printf("%s\n",stage->opcode);

      
      
      if (STOP_INDEX != 0 || MUL_INDEX != 0) {
          cpu->pc -=4;
      } else{
          
          /* Copy data from fetch latch to decode latch*/
          cpu->stage[DRF] = cpu->stage[F];
      }
      
      //HALT change to NOP
      if (strcmp(stage_MEM->opcode, "HALT") == 0) {
          strcpy(stage->opcode, "NOP");
          stage->rd=-1;
      }
//
//      if (MUL_INDEX != 0) {
//          cpu->pc -=4;
//      }
      
    if (ENABLE_DEBUG_MESSAGES) {
        if (Total_type == 2) {
            
            print_stage_content("Fetch", stage);
        }
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
    
//    CPU_Stage* stage_F = &cpu->stage[F];
    CPU_Stage* stage_EX = &cpu->stage[EX];
    CPU_Stage* stage_MEM = &cpu->stage[MEM];
    CPU_Stage* stage_WB = &cpu->stage[WB];
    
    
  if (!stage->busy && !stage->stalled) {

      //BZ change to NOP
      if (strcmp(stage_WB->opcode, "BZ") == 0) {
          if (cpu->z_flag[0] != 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
          }
      }
      
      //BNZ change to NOP
      if (strcmp(stage_WB->opcode, "BNZ") == 0) {
          if (cpu->z_flag[0] == 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
          }
      }
      
      //JUMP change to NOP
      if (strcmp(stage_WB->opcode, "JUMP") == 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
      }
      
      //HALT change to NOP
      if (strcmp(stage_WB->opcode, "HALT") == 0) {
          strcpy(stage->opcode, "NOP");
          stage->rd=-1;
      }
      
      //HALT change to NOP
      if (strcmp(stage_MEM->opcode, "HALT") == 0) {
          strcpy(stage->opcode, "NOP");
          stage->rd=-1;
      }
      
      
    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
        if (stage->rs1 == stage_EX->rd) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = stage_EX->buffer;
                RS1_INDEX=0;
//                stage->rs2_value = cpu->regs[stage->rs2];
//
//                /* Copy data from decode latch to execute latch*/
//                cpu->stage[EX] = cpu->stage[DRF];
//                STOP_INDEX=0;
            }

            
        } else if (stage->rs1 == stage_MEM->rd){
            if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                //
                RS1_INDEX=1;
//                strcpy(stage_EX->opcode, "NOP");
//                //          stage_EX->pc = 0;
//
//                stage_EX->rd=-1;
            }else{
                
                stage->rs1_value = stage_MEM->buffer;
                RS1_INDEX=0;
//                stage->rs2_value = cpu->regs[stage->rs2];
//
//                /* Copy data from decode latch to execute latch*/
//                cpu->stage[EX] = cpu->stage[DRF];
//                STOP_INDEX=0;
            }

            
        } else if (stage->rs1 == stage_WB->rd){
            
            stage->rs1_value = stage_WB->buffer;
            RS1_INDEX=0;
//            stage->rs2_value = cpu->regs[stage->rs2];
//
//            /* Copy data from decode latch to execute latch*/
//            cpu->stage[EX] = cpu->stage[DRF];
//            STOP_INDEX=0;
            
        } else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
        }
            
            
        if (stage->rs2 == stage_EX->rd) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                
                stage->rs2_value = stage_EX->buffer;
                RS2_INDEX=0;
            }
            
        } else if (stage->rs2 == stage_MEM->rd){
            if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                RS2_INDEX=0;
                stage->rs2_value = stage_MEM->buffer;
            }
            
        } else if (stage->rs2 == stage_WB->rd){
            
            RS2_INDEX=0;
            stage->rs2_value = stage_WB->buffer;
            
        }else{
            RS2_INDEX=0;
            stage->rs2_value = cpu->regs[stage->rs2];
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        if (STOP_INDEX != 0) {
            strcpy(stage_EX->opcode, "NOP");
            //          stage_EX->pc = 0;
            
            stage_EX->rd=-1;
        }
        else if (MUL_INDEX != 0){
            
        }
        else {
            
            /* Copy data from decode latch to execute latch*/
            cpu->stage[EX] = cpu->stage[DRF];
        }
        
    }

      /* Read data from register file for load */
      if (strcmp(stage->opcode, "LOAD") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") != 0) {
                  RS1_INDEX=1;
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
                  
          } else if (stage->rs1 == stage_WB->rd){
              
              stage->rs1_value = stage_WB->buffer;
              
              RS1_INDEX=0;

          }else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
          }
          
          if (RS1_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              stage_EX->rd=-1;
              
          }
          else if (MUL_INDEX != 0){
              
          } else {
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];          }
      }
      
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        //no instruction needed for movc
        
        if (MUL_INDEX == 0){
            /* Copy data from decode latch to execute latch*/
            cpu->stage[EX] = cpu->stage[DRF];
        }
    }
      
      /* No Register file read needed for ADD */
      if (strcmp(stage->opcode, "ADD") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
              
          } else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
              
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
                  
              }
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          } else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if(MUL_INDEX != 0){
              
          } else {
              
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* No Register file read needed for SUB */
      if (strcmp(stage->opcode, "SUB") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
          }else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
              
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
                  
              }
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
                  
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          }else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if (MUL_INDEX != 0){
              
          } else {
            
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* Read data from register file for AND */
      if (strcmp(stage->opcode, "AND") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
              
          }else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          }else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if (MUL_INDEX != 0){
              
          } else {
              
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* Read data from register file for OR */
      if (strcmp(stage->opcode, "OR") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
              
          } else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
              }
              stage_EX->rd=-1;
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          }else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if (MUL_INDEX != 0){
              
          } else {
              
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* Read data from register file for EX-OR */
      if (strcmp(stage->opcode, "EX-OR") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
              
          } else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
              
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          }else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if (MUL_INDEX != 0){
              
          } else {
              
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* No Register file read needed for MUL */
      if (strcmp(stage->opcode, "MUL") == 0) {
          if (stage->rs1 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_EX->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  RS1_INDEX=1;
                  
              }else{
                  
                  stage->rs1_value = stage_MEM->buffer;
                  RS1_INDEX=0;
              }
              
          } else if (stage->rs1 == stage_WB->rd){
              stage->rs1_value = stage_WB->buffer;
              RS1_INDEX=0;
              
          } else{
              stage->rs1_value = cpu->regs[stage->rs1];
              RS1_INDEX=0;
              
          }
          
          if (stage->rs2 == stage_EX->rd) {
              if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_EX->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_MEM->rd){
              if (strcmp(stage_MEM->opcode, "LOAD") == 0) {
                  
                  RS2_INDEX=1;
              }else{
                  
                  stage->rs2_value = stage_MEM->buffer;
                  RS2_INDEX=0;
              }
              
          } else if (stage->rs2 == stage_WB->rd){
              
              stage->rs2_value = stage_WB->buffer;
              RS2_INDEX=0;
              
          }else{
              stage->rs2_value = cpu->regs[stage->rs2];
              RS2_INDEX=0;
          }
          
          if (RS1_INDEX == 0 && RS2_INDEX == 0) {
              STOP_INDEX=0;
          }else{
              STOP_INDEX=1;
          }
          
          
          if (STOP_INDEX != 0) {
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
          }
          else if (MUL_INDEX != 0){
              
          } else {
              
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }
      }
      
      /* Read data from register file for BZ */
      if (strcmp(stage->opcode, "BZ") == 0) {
          
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          
      }
      
      /* Read data from register file for BNZ */
      if (strcmp(stage->opcode, "BNZ") == 0) {
          
          
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          
      }
      
      /* Read data from register file for JUMP */
      if (strcmp(stage->opcode, "JUMP") == 0) {
         stage->rs1_value=cpu->regs[stage->rs1]; 
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          
      }
      
      /* No Register file read needed for HALT */
      if (strcmp(stage->opcode, "HALT") == 0) {
          //no instruction needed for nop

              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
      }
      
      /* No Register file read needed for NOP */
      if (strcmp(stage->opcode, "NOP") == 0) {
          //no instruction needed for nop
          
//          printf("%d \n",MUL_INDEX);
          if (MUL_INDEX == 0){
              /* Copy data from decode latch to execute latch*/
              cpu->stage[EX] = cpu->stage[DRF];
          }

      }

    if (ENABLE_DEBUG_MESSAGES) {
        if (Total_type == 2) {
            print_stage_content("Decode/RF", stage);
        }
      
    }
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX];
//    CPU_Stage* stage_F = &cpu->stage[F];
//    CPU_Stage* stage_DRF = &cpu->stage[DRF];
    CPU_Stage* stage_MEM = &cpu->stage[MEM];
    CPU_Stage* stage_WB = &cpu->stage[WB];
    
  if (!stage->busy && !stage->stalled) {

      //BZ change to NOP
      if (strcmp(stage_WB->opcode, "BZ") == 0) {
          if (cpu->z_flag[0] != 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
          }
      }
      
      //BNZ change to NOP
      if (strcmp(stage_WB->opcode, "BNZ") == 0) {
          if (cpu->z_flag[0] == 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
          }
      }
      
      //JUMP change to NOP
      if (strcmp(stage_WB->opcode, "JUMP") == 0) {
              strcpy(stage->opcode, "NOP");
              stage->rd=-1;
      }
      
      //HALT change to NOP
      if (strcmp(stage_WB->opcode, "HALT") == 0) {
          strcpy(stage->opcode, "NOP");
          stage->rd=-1;
      }
      
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
        stage->buffer = stage->rs2_value + stage->imm;
        
        /* Copy data from Execute latch to Memory latch*/
        cpu->stage[MEM] = cpu->stage[EX];
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        stage->buffer = stage->imm + 0;
        
        /* Copy data from Execute latch to Memory latch*/
        cpu->stage[MEM] = cpu->stage[EX];
    }
      
      /* LOAD  */
      if (strcmp(stage->opcode, "LOAD") == 0) {
          stage->buffer = stage->rs1_value + stage->imm;
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* ADD  */
      if (strcmp(stage->opcode, "ADD") == 0) {
          stage->buffer = stage->rs1_value + stage->rs2_value;
          
          //z_flag
          if (stage->buffer == 0) {
              cpu->z_flag[0] = 1;
          } else{
              cpu->z_flag[0] = 0;
          }
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* SUB  */
      if (strcmp(stage->opcode, "SUB") == 0) {
          stage->buffer = stage->rs1_value - stage->rs2_value;
          
          //z_flag
          if (stage->buffer == 0) {
              cpu->z_flag[0] = 1;
          } else{
              cpu->z_flag[0] = 0;
          }
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* AND  */
      if (strcmp(stage->opcode, "AND") == 0) {
          stage->buffer = stage->rs1_value & stage->rs2_value;
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* OR  */
      if (strcmp(stage->opcode, "OR") == 0) {
          stage->buffer = stage->rs1_value | stage->rs2_value;
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* EX-OR  */
      if (strcmp(stage->opcode, "EX-OR") == 0) {
          stage->buffer = stage->rs1_value ^ stage->rs2_value;
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* MUL  */
      if (strcmp(stage->opcode, "MUL") == 0) {
//        printf("%d \n",MUL_INDEX);
          if (MUL_INDEX == 0) {

              
              cpu->stage[DRF] = cpu->stage[F];
//              STOP_INDEX=1;
              strcpy(stage_MEM->opcode, "NOP");
              stage_MEM->rd=0;
              MUL_INDEX = 1;
          } else {
//              STOP_INDEX=0;
              stage->buffer = stage->rs1_value * stage->rs2_value;
              MUL_INDEX = 0;
              
              //z_flag
              if (stage->buffer == 0) {
                  cpu->z_flag[0] = 1;
              } else{
                  cpu->z_flag[0] = 0;
              }
              
              /* Copy data from Execute latch to Memory latch*/
              cpu->stage[MEM] = cpu->stage[EX];
          }

         
//          stage->buffer = stage->rs1_value * stage->rs2_value;
      }
      
      /* BZ  */
      if (strcmp(stage->opcode, "BZ") == 0) {
          stage->buffer = stage->pc + stage->imm;
          
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* BNZ  */
      if (strcmp(stage->opcode, "BNZ") == 0) {
          stage->buffer = stage->pc + stage->imm;
          
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* JUMP  */
      if (strcmp(stage->opcode, "JUMP") == 0) {
          stage->buffer = stage->rs1_value + stage->imm;
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      
      /* No Register file read needed for HALT */
      if (strcmp(stage->opcode, "HALT") == 0) {
          //no instruction needed for nop
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
      /* No Register file read needed for NOP */
      if (strcmp(stage->opcode, "NOP") == 0) {
          //no instruction needed for nop
          
          /* Copy data from Execute latch to Memory latch*/
          cpu->stage[MEM] = cpu->stage[EX];
      }
      
//      if (cpu->ins_completed == (cpu->code_memory_size-2)) {
//          strcpy(stage->opcode,"NOP");
//      }


    if (ENABLE_DEBUG_MESSAGES) {
        if (Total_type == 2) {
            print_stage_content("Execute", stage);
        }
      
    }
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM];
    
    CPU_Stage* stage_DRF = &cpu->stage[DRF];
    CPU_Stage* stage_EX = &cpu->stage[EX];
    
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
        stage->mem_address = stage->buffer;
        cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }
      
      /* LOAD */
      if (strcmp(stage->opcode, "LOAD") == 0) {
          stage->mem_address = stage->buffer;
          stage->rs2_value = cpu->data_memory[stage->mem_address];
          stage->buffer=stage->rs2_value;
      }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        //no instructions in memory
    }
      
      /* ADD */
      if (strcmp(stage->opcode, "ADD") == 0) {
          //no instructions in memory
          
      }
      
      /* SUB */
      if (strcmp(stage->opcode, "SUB") == 0) {
          //no instructions in memory
          
      }

      /* AND */
      if (strcmp(stage->opcode, "AND") == 0) {
          //no instructions in memory
          
      }
      
      /* OR */
      if (strcmp(stage->opcode, "OR") == 0) {
          //no instructions in memory
          
      }
      
      /* EX-OR */
      if (strcmp(stage->opcode, "EX-OR") == 0) {
          //no instructions in memory
          
      }
      
      /* MUL */
      if (strcmp(stage->opcode, "MUL") == 0) {
          //no instructions in memory
          
      }
      
      /* BZ */
      if (strcmp(stage->opcode, "BZ") == 0) {
          //no instructions in memory
          if (cpu->z_flag[0] == 1) {
              MUL_INDEX =0;
              STOP_INDEX=0;
              
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
              
              strcpy(stage_DRF->opcode, "NOP");
              //          stage_DRF->pc = 0;
              stage_DRF->rd=-1;
              
              cpu->pc = stage->buffer;
          }
        

      }
      
      /* BNZ */
      if (strcmp(stage->opcode, "BNZ") == 0) {
          //no instructions in memory
          if (cpu->z_flag[0] == 0) {
              MUL_INDEX =0;
              STOP_INDEX=0;
              
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
              
              strcpy(stage_DRF->opcode, "NOP");
              //          stage_DRF->pc = 0;
              stage_DRF->rd=-1;
              
              cpu->pc = stage->buffer;
          }
          
          
      }
      
      /* JUMP */
      if (strcmp(stage->opcode, "JUMP") == 0) {
          //no instructions in memory
              MUL_INDEX =0;
              STOP_INDEX=0;
              
              strcpy(stage_EX->opcode, "NOP");
              //          stage_EX->pc = 0;
              
              stage_EX->rd=-1;
              
              strcpy(stage_DRF->opcode, "NOP");
              //          stage_DRF->pc = 0;
              stage_DRF->rd=-1;
              
              cpu->pc = stage->buffer;
      }
      
      /* HALT */
      if (strcmp(stage->opcode, "HALT") == 0) {
          //no instructions in memory
         
      }
      
    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
        if (Total_type == 2) {
            print_stage_content("Memory", stage);
        }
      
    }
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        cpu->regs_valid[stage->rd] = 1;
        cpu->regs[stage->rd] = stage->buffer;
        
        cpu->ins_completed++;
    }
      /* Store */
      if (strcmp(stage->opcode, "STORE") == 0) {
          cpu->ins_completed++;
      }
      
      /* LOAD */
      if (strcmp(stage->opcode, "LOAD") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->rs2_value;
          
          cpu->ins_completed++;
      }
      
      /* ADD */
      if (strcmp(stage->opcode, "ADD") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* SUB */
      if (strcmp(stage->opcode, "SUB") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* AND */
      if (strcmp(stage->opcode, "AND") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* OR */
      if (strcmp(stage->opcode, "OR") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* EX-OR */
      if (strcmp(stage->opcode, "EX-OR") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* MUL */
      if (strcmp(stage->opcode, "MUL") == 0) {
          cpu->regs_valid[stage->rd] = 1;
          cpu->regs[stage->rd] = stage->buffer;
          
          cpu->ins_completed++;
      }
      
      /* BZ */
      if (strcmp(stage->opcode, "BZ") == 0) {
          cpu->ins_completed++;
      }
      
      /* BNZ */
      if (strcmp(stage->opcode, "BNZ") == 0) {
          cpu->ins_completed++;
      }
      
      /* JUMP */
      if (strcmp(stage->opcode, "JUMP") == 0) {
          cpu->ins_completed++;
      }
      
      /* HALT */
      if (strcmp(stage->opcode, "HALT") == 0) {
          HALT_INDEX = 1;
      }

      /* NOP */
      if (strcmp(stage->opcode, "NOP") == 0) {
          
      }

    if (ENABLE_DEBUG_MESSAGES) {
        if (Total_type == 2) {
            print_stage_content_WB("Writeback", stage);
        }
      
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->pc == (4000+ 4*(cpu->code_memory_size+4)) || cpu->clock == Total_cycle) {
      printf("(apex) >> Simulation Complete");
//        printf("\n%d \n",cpu->code_memory_size);
//        printf("%d \n",cpu->ins_completed);
      break;
    }
      
      if (Total_type == 2) {
          if (ENABLE_DEBUG_MESSAGES) {
              printf("--------------------------------\n");
              printf("Clock Cycle #: %d\n", (cpu->clock+1));
              printf("--------------------------------\n");
          }
      }


    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
      
      if (HALT_INDEX != 0) {
          break;
      }
  }
    
    if (Total_type != 0) {
        printf("\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");
        
        for (int a=0; a<16; a++) {
            char str[10];
            if (cpu->regs_valid[a] == 1) {
                strcpy(str, "Valid");
            } else{
                strcpy(str, "Invalid");
            }
            
            if (a<10) {
                printf("|   REG[0%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->regs[a], str);
            } else{
                printf("|   REG[%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->regs[a], str);
            }
        }
        
        printf("\n============== STATE OF DATA MEMORY =============\n");
        
        for (int a=0; a<100; a++) {
            
            if (a<10) {
                printf("|   MEM[0%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
            } else{
                printf("|   REG[%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
            }
        }
        
    }
    
    
  return 0;
}
