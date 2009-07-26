
command :compare do |c|
  c.syntax = 'corpus compare DIR CATEGORY'
  c.description = 'Use the CoNLL eval script to compare a baseline against the system output, in order to determine the effect of the introduced feature.'
  c.option '-p', 'Do not score punctuation'
  c.when_called do |args, options|
    if args.size < 1
      say "Missing corpus directory"
    else
      punc = options.p ? '-p': ''
      dir = args[0]
      cat = args[1] || ''
      gold = Dir.glob("#{dir}/test/#{cat}/*.conll").first
      say "[gold] #{gold}"
      contenders = %w{baseline system}.collect { |name|
        Dir.glob("#{dir}/#{name}/#{cat}/*.conll").first
      }
      evalbs = []
      for file in contenders
        say "[eval] #{file}"
        puts `bin/eval.pl #{punc} -q -g #{gold} -s #{file} 2> /dev/null`
      end
      evalbs = contenders.collect { |file|
        evalb = File.dirname(file) + "/evalb.out"
        say "[evalb] #{file} => #{evalb}"
        `bin/eval07.pl #{punc} -b -q -g #{gold} -s #{file} > #{evalb} 2> /dev/null`
        evalb
      }
      say "[compare] #{evalbs[0]} #{evalbs[1]}"
      puts `bin/compare.pl #{evalbs[0]} #{evalbs[1]} 2> /dev/null`
    end
  end
end