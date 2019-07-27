project('csdc5_cdh', 'c', 'cpp')

freertos_src = [
  'packages/freertos/list.c',
  'packages/freertos/queue.c',
  'packages/freertos/stream_buffer.c',
  'packages/freertos/tasks.c',
  'packages/freertos/timers.c',
]
static_library(
  'freertos',
  freertos_src,
  include_directories : [
    'config',
    'packages/freertos/include',
    'packages/freertos/portable/GCC/ARM_CM7/r0p1',
  ]
)

sthal_src = [
  
]
