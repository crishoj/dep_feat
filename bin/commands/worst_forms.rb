
command :worst_forms do |c|
  c.option '--threshold N', Integer, 'Only consider forms occurring at least N times'
  c.option '--take N', Integer, 'Show the N worst scoring forms'
  c.when_called do |args,options|
    options.default(:threshold => 5, :take => 10)
    gold =   Conll::Corpus.parse find_corpus_file(args[0], 'test')
    system = Conll::Corpus.parse find_corpus_file(args[0], 'system')
    la_scores = { 'OVERALL' => [] }
    say "Collecting scores from #{system.sentences.length} sentences"
    progress system.sentences do |sent|
      uas, las = sent.score(gold[sent.index])
      sent.tokens.forms.uniq.each do |form|
        la_scores[form] ||= []
        la_scores[form].push las
      end
      la_scores['OVERALL'].push las
    end
    la_avgs = {}
    say "Averaging scores for #{la_scores.length} forms"
    progress la_scores.keys do |form|
      occurrences = la_scores[form].size
      next if occurrences < options.threshold
      la_avgs[form] = la_scores[form].sum / occurrences
    end
    overall = la_avgs.delete 'OVERALL'
    t = table ['Form', 'LAS']
    say "Sorting #{la_avgs.length} averages"
    la_avgs.sort_by(&:last)[0 .. options.take].each do |row|
      t.add_row row
    end
    t.add_row ['OVERALL', overall]
    say "#{options.take} worst scoring forms"
    puts t
  end
end