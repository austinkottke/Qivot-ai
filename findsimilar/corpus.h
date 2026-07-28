#ifndef CORPUS_H
#define CORPUS_H

// The little "library" we search. Eight short notes, each about a different
// everyday thing, with their own distinctive words. Swap in your own notes,
// recipes, emails — anything — and the search still works.
struct DocDef { const char *title; const char *text; };

static const DocDef kDocs[] = {
    { "Training a puppy",
      "Teach your new puppy to sit, stay, and come. Reward the dog with a treat "
      "and lots of praise. Short daily practice builds good habits." },
    { "Baking bread",
      "Mix flour, water, yeast, and salt into a dough. Let it rise until puffy, "
      "then bake in a hot oven until the crust turns golden brown." },
    { "The night sky",
      "On a clear night you can see stars, planets, and the moon. A telescope "
      "reveals craters, comets, and distant galaxies far out in deep space." },
    { "Brewing coffee",
      "Grind the beans, pour in hot water, and let the coffee drip slowly. A good "
      "morning cup is rich, dark, and full of warm aroma." },
    { "Mountain hiking",
      "Pack water and sturdy boots for the trail. The steep climb rewards you "
      "with fresh air and a wide view from the rocky summit." },
    { "Sailing the ocean",
      "Raise the sail and catch the wind across the open sea. Watch the waves and "
      "steer the boat toward the distant harbor at sunset." },
    { "Growing tomatoes",
      "Plant the seeds in rich soil and water them in the sun. Through summer the "
      "green vines slowly fill with ripe red tomatoes." },
    { "Playing guitar",
      "Press the strings onto the frets and strum a chord. Daily practice trains "
      "your fingers to play songs and smooth melodies." },
};

static const int kDocCount = int(sizeof(kDocs) / sizeof(kDocs[0]));

#endif // CORPUS_H
