childEffectsCount = 1
childEffectsCost = {{0,1,0,0}}
childEffectsIcon = {"magiccircle"}
icon = "magiccircle"
isCumulative = 1

function getName()
	if language == "french" then
		return "Cercle de magie"
	else
		return "Magic circle"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Cercle de magie"
	else
		return "Magic circle"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 1|. Place un cercle de magie sur la carte, afin de pouvoir y lancer des sorts. Si vous aviez déjà placé un cercle de cette manière, le précédent est détruit."
	else
		return "Activation cost: 1|. Put a magic circle on the map, in order to cast spells from there. If you already put a circle with this skill, the previous one is destroyed."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("tile", "inrange")
	end
end

currentCircle = { -1, -1 } -- player id, circle id

function onResolveChild(effect, params, caster)
	if effect == 1 then
		if currentCircle[2] >= 0 then
			removeMagicCircle(currentCircle[1], currentCircle[2])
		end
		x, y = splitint(params, " ");
		currentCircle[1] = caster
		currentCircle[2] = addMagicCircle(caster, x, y)
	end
end
