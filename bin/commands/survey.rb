# encoding: UTF-8

command :survey do |c|
  c.when_called do |args, options|
    categories = {
      'quotes'      => { :form_re => /"|»|``/ },
      'parenthesis' => { :form => '(' },
      'colon'       => { :form => ':' },
      'hypen'       => { :form => '-' }
    }
    Dir.glob("#{args[0]}/test/*.conll").each do |file|
      gold   = Conll::Corpus.parse(file)
      system = Conll::Corpus.parse(file.sub(/test\/.+$/, 'system/out.conll'))
      measure(file, system, gold, categories)
    end
  end
end
