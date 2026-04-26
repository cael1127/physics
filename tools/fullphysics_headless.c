#include "fullphysics.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CliArgs {
  int demo_id;
  int steps;
  float dt;
  int vel_iters;
  int pos_iters;
  int disable_fluid;
  int disable_particles;
  float wind_x;
  float wind_y;
  float gravity_scale;
  const char* csv_path;
  const char* snapshot_out;
} CliArgs;

static void cli_defaults(CliArgs* a) {
  memset(a, 0, sizeof(*a));
  a->demo_id = FP_SCENE_DEMO_STACK;
  a->steps = 600;
  a->dt = 1.0f / 60.0f;
  a->vel_iters = 10;
  a->pos_iters = 4;
  a->disable_fluid = 1;
  a->disable_particles = 1;
  a->gravity_scale = 1.0f;
}

static int parse_int_arg(const char* s, int* out) {
  char* end = NULL;
  long v = strtol(s, &end, 10);
  if (!end || *end != '\0') return 0;
  *out = (int)v;
  return 1;
}

static int parse_float_arg(const char* s, float* out) {
  char* end = NULL;
  float v = strtof(s, &end);
  if (!end || *end != '\0') return 0;
  *out = v;
  return 1;
}

static int parse_cli(int argc, char** argv, CliArgs* out) {
  cli_defaults(out);
  for (int i = 1; i < argc; i++) {
    const char* k = argv[i];
    if (strcmp(k, "--demo") == 0 && i + 1 < argc) {
      if (!parse_int_arg(argv[++i], &out->demo_id)) return 0;
    } else if (strcmp(k, "--steps") == 0 && i + 1 < argc) {
      if (!parse_int_arg(argv[++i], &out->steps)) return 0;
    } else if (strcmp(k, "--dt") == 0 && i + 1 < argc) {
      if (!parse_float_arg(argv[++i], &out->dt)) return 0;
    } else if (strcmp(k, "--vel-iters") == 0 && i + 1 < argc) {
      if (!parse_int_arg(argv[++i], &out->vel_iters)) return 0;
    } else if (strcmp(k, "--pos-iters") == 0 && i + 1 < argc) {
      if (!parse_int_arg(argv[++i], &out->pos_iters)) return 0;
    } else if (strcmp(k, "--wind-x") == 0 && i + 1 < argc) {
      if (!parse_float_arg(argv[++i], &out->wind_x)) return 0;
    } else if (strcmp(k, "--wind-y") == 0 && i + 1 < argc) {
      if (!parse_float_arg(argv[++i], &out->wind_y)) return 0;
    } else if (strcmp(k, "--gravity-scale") == 0 && i + 1 < argc) {
      if (!parse_float_arg(argv[++i], &out->gravity_scale)) return 0;
    } else if (strcmp(k, "--enable-fluid") == 0) {
      out->disable_fluid = 0;
    } else if (strcmp(k, "--enable-particles") == 0) {
      out->disable_particles = 0;
    } else if (strcmp(k, "--csv") == 0 && i + 1 < argc) {
      out->csv_path = argv[++i];
    } else if (strcmp(k, "--snapshot-out") == 0 && i + 1 < argc) {
      out->snapshot_out = argv[++i];
    } else if (strcmp(k, "--help") == 0) {
      return 0;
    } else {
      fprintf(stderr, "Unknown arg: %s\n", k);
      return 0;
    }
  }
  return 1;
}

static float compute_kinetic_energy(const Fp2World* w) {
  float e = 0.0f;
  for (int i = 0; i < w->body_count; i++) {
    const Fp2Body* b = &w->bodies[i];
    if (b->is_static) continue;
    float m = b->inv_mass > 0.0f ? (1.0f / b->inv_mass) : 0.0f;
    float I = b->inv_inertia > 0.0f ? (1.0f / b->inv_inertia) : 0.0f;
    float v2 = fp_v2_len2(b->v);
    e += 0.5f * m * v2 + 0.5f * I * b->w * b->w;
  }
  return e;
}

static float compute_com_y(const Fp2World* w) {
  float mass_sum = 0.0f;
  float y_sum = 0.0f;
  for (int i = 0; i < w->body_count; i++) {
    const Fp2Body* b = &w->bodies[i];
    if (b->is_static) continue;
    float m = b->inv_mass > 0.0f ? (1.0f / b->inv_mass) : 0.0f;
    mass_sum += m;
    y_sum += m * b->p.y;
  }
  if (mass_sum <= 1e-6f) return 0.0f;
  return y_sum / mass_sum;
}

int main(int argc, char** argv) {
  CliArgs args;
  if (!parse_cli(argc, argv, &args)) {
    printf("Usage: fullphysics_headless [--demo N] [--steps N] [--dt F] [--vel-iters N] [--pos-iters N]\n");
    printf("                          [--wind-x F] [--wind-y F] [--gravity-scale F]\n");
    printf("                          [--enable-fluid] [--enable-particles]\n");
    printf("                          [--csv path.csv] [--snapshot-out file.fp2snap]\n");
    return 1;
  }

  FpSceneDesc desc;
  fp_scene_desc_default(&desc);
  if (args.disable_fluid) desc.enable_fluid = 0;
  if (args.disable_particles) desc.enable_particles = 0;

  FpScene scene;
  fp_scene_init(&scene, &desc);
  fp_scene_load_demo(&scene, args.demo_id);
  scene.params.wind = fp_v2(args.wind_x, args.wind_y);
  scene.params.gravity_scale = args.gravity_scale;

  FILE* csv = NULL;
  if (args.csv_path) {
    csv = fopen(args.csv_path, "w");
    if (!csv) {
      fprintf(stderr, "Failed to open CSV output: %s\n", args.csv_path);
      fp_scene_destroy(&scene);
      return 2;
    }
    fprintf(csv, "step,time,kinetic_energy,com_y,contact_count,body_count\n");
  }

  for (int i = 0; i < args.steps; i++) {
    fp_scene_step(&scene, args.dt, args.vel_iters, args.pos_iters);
    if (csv) {
      const Fp2World* w = &scene.world;
      float t = (float)(i + 1) * args.dt;
      float ke = compute_kinetic_energy(w);
      float com_y = compute_com_y(w);
      fprintf(
          csv,
          "%d,%.6f,%.9g,%.9g,%d,%d\n",
          i + 1,
          (double)t,
          (double)ke,
          (double)com_y,
          w->contact_count,
          w->body_count);
    }
  }

  if (csv) fclose(csv);
  if (args.snapshot_out && !fp_scene_save_snapshot(&scene, args.snapshot_out)) {
    fprintf(stderr, "Failed to save snapshot: %s\n", args.snapshot_out);
    fp_scene_destroy(&scene);
    return 3;
  }

  printf(
      "headless_run_ok demo=%d steps=%d dt=%.6f contacts=%d bodies=%d joints=%d revolute=%d\n",
      args.demo_id,
      args.steps,
      (double)args.dt,
      scene.world.contact_count,
      scene.world.body_count,
      scene.world.joint_count,
      scene.world.revolute_joint_count);

  fp_scene_destroy(&scene);
  return 0;
}
