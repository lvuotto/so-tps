#!/usr/bin/env ruby

semilla = 1
cantidad = 20
tareas = 8

1.upto(tareas) do
  puts "TaskCPU #{10 + rand(cantidad)}"
end

# semilla = 1
# cantidad = 100
# tareas = 8
# 
# if ARGV.length == 3 then
#   semilla = ARGV[0].to_i
#   semilla = ARGV[1].to_i
#   semilla = ARGV[2].to_i
# end
# 
# srand semilla
# tasks = ["TaskCPU", "TaskIO", "TaskConsola"]
# 
# 1.upto(tareas) do
#   puts "@#{rand cantidad/10}"
#   task = tasks[rand tasks.length]
#   case task
#     when "TaskCPU"
#       puts "#{task} #{1 + rand(cantidad)}"
#     when "TaskIO"
#       puts "#{task} #{1 + rand(cantidad)}"
#     when "TaskConsola"
#       bmin = rand cantidad
#       bmax = rand cantidad
#       bmin, bmax = bmax, bmin if bmin > bmax
#       puts "#{task} #{1 + rand(cantidad)} #{bmin} #{bmax}"
#   end
# end
