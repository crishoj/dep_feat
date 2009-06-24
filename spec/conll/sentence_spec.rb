
require 'spec/conll/spec_helper'
require 'lib/conll/sentence'
require 'lib/conll/corpus'

describe Conll::Sentence do
  before(:all) do
    @corpus_parts = []
    File.new(@corpus_filename).each("\n\n") do |part|
      @corpus_parts << part
    end
    @sentence_lines = @corpus_parts[0].split(/\n/)
  end

  before(:each) do
    @sentence = Conll::Sentence.parse(@sentence_lines)
  end

  it "should have 22 tokens" do
    @sentence.should have(22).tokens
  end

  it "should interpret dependencies" do
    @sentence.tokens.first.head.should_not be_nil
  end

  it "should interpret dependencies correctly" do
    @sentence.tokens.first.head.form.should match(/tror/)
  end

end

