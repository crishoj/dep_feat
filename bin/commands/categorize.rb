
command :categorize do |c|
  c.syntax = 'corpus categorize --kind ANNOTATOR DIR'
  c.description = 'Split corpus into several parts according to the presence of a feature. This is useful for evaluating the effect of introducing the feature.'
  c.when_called do |args, options|
    if not options.kind
      say "Missing annotator class"
    elsif args.size == 0
      say "Missing corpus directory"
    elsif not File.directory?(args[0])
      say "Invalid corpus directory"
    else
      dir  = args[0]
      gold_file = Dir.glob("#{dir}/test/*.conll").first
      say "      [gold] #{gold_file}"
      gold = Conll::Corpus.parse gold_file
      annotator = Annotation::find_annotator(options.kind).new
      %w{baseline test system}.each do |section|
        filename = Dir.glob("#{dir}/#{section}/*.conll").first
        corpus = Conll::Corpus.parse filename
        say "[categorize] #{filename}"
        annotator.categorize(gold, corpus).each_pair do |category, part|
          category_filename = filename.sub(/#{section}/, "#{section}/#{category}")
          make_directory File.dirname(category_filename)
          say "      [part] #{category} => #{category_filename}"
          File.open(category_filename, 'w') do |f|
            f << part
          end
        end
      end
    end
  end
end
