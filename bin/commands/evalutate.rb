
command :evaluate do |c|
  c.syntax = 'corpus evaluate [options] DIR'
  c.description = 'Gather performance metrics related to the specified KIND of augmented feature'
  c.when_called do |args, options|
    options.default :kind => 'null'
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      dir  = args[0]
      gold     = Conll::Corpus.parse Dir.glob("#{dir}/test/*.conll").first
      baseline = Conll::Corpus.parse Dir.glob("#{dir}/baseline/*.conll").first
      system   = Conll::Corpus.parse Dir.glob("#{dir}/system/*.conll").first
      measure 'gold',     gold,     gold
      measure 'baseline', baseline, gold
      measure 'system',   system,   gold
    else
      say "Invalid corpus directory"
    end
  end
end
