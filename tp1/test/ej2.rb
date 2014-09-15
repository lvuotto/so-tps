#!/usr/bin/env ruby

semilla = 1
max = 100

if ARGV.length == 2 then
  semilla = ARGV[0].to_i
  max = ARGV[1].to_i
end

srand semilla

puts "@#{rand max/10}"
puts "TaskCPU #{rand max}"
1.upto(2) do
  bmin = rand max
  bmax = rand max
  if bmin > bmax then
    bmin, bmax = bmax, bmin
  end
  puts "@#{rand max/10}"
  puts "TaskConsola #{rand max/10} #{bmin} #{bmax}"
end
