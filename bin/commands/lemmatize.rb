
command :lemmatize do |c|
  c.syntax = 'corpus lemmatize DIR'
  c.description = 'Make a copy of DIR and lemmatize each of its corpus files using the CST lemmatizer (Danish only).'
  c.when_called do |args, options|
    base = 'vendor/cstlemma'
    dir = args[0]
    target_dir = "#{dir}-lemmatized"
    make_directory(target_dir)
    Dir.glob("#{dir}/*/*.conll").each do |source|
      target = source.gsub(dir, target_dir)
      target_subdir = File.dirname(target)
      make_directory(target_subdir)
      tmp_file = target.sub(/conll/, 'forms')
      say "   [extract] #{source} -> #{tmp_file}"
      `cat #{source} | awk '{print $2 "/" $5}' > #{tmp_file}`
      lemma_file = "#{tmp_file}.lemma"
      say " [lemmatize] #{tmp_file} -> #{lemma_file}"
      `#{base}/bin/cstlemma -L -t+ -f #{base}/flexrules.UTF8 -d #{base}/dict.UTF8 -i #{tmp_file}`
      say " [recombine] #{source} + #{lemma_file} > #{target}"
      `paste #{source} #{lemma_file} | awk -F"\t" 'OFS="\t" { print $1, $2, $12, $4, $5, $6, $7, $8, $9, $10 }' > #{target}`
      say "   [cleanup] #{tmp_file}, #{lemma_file}"
      File.delete(tmp_file)
      File.delete(lemma_file)
    end

  end
end