
command :eval do |c|
  c.syntax = 'corpus eval DIR'
  c.description = 'Run the CoNLL eval script on the given corpus'
  c.when_called do |args, options|
    system = Dir.glob("#{args[0]}/system/*.conll").first
    gold   = Dir.glob("#{args[0]}/test/*.conll").first
    say `bin/eval.pl -g #{gold} -s #{system}`
  end
end