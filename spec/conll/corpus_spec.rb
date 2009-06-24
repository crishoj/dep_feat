
require 'spec/conll/spec_helper'
require 'lib/conll/corpus'

describe Conll::Corpus do
  before(:each) do
    @corpus = Conll::Corpus.parse(@corpus_filename)
  end

  it "should have 4 sentences" do
    @corpus.should have(4).sentences
  end
end

