#!/usr/bin/env ruby

cantidad = 40 # cantidad de tareas * quantum = 8 * 5
ticks_tareas = [0, 0, 0, 0, 0, 0, 0, 0]
archivos = []
1.upto(1000) do |i|
  archivos << "benchmark/ej9-1-q-5-s-#{i}.dat"
end

archivos.each do |archivo|
  f = open archivo
  i = 1
  f.each_line do |l|
    if not l.start_with? "#" and l.include? "CPU" then
      cpu, tick, tarea, core = l.split " "
      next if tarea.to_i == -1
      ticks_tareas[tarea.to_i] += 1
      i += 1
    end
    break if i >= cantidad
  end
end

i = 0
ticks_tareas.each do |t|
  puts "Tarea #{i}: #{t}"
  i += 1
end
