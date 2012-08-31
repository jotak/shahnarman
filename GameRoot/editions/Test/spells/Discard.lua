cost = {0,0,2,0}
icon = "frozen_land"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Défausse"
	else
		return "Discard"
	end
end

function getDescription()
	if language == "french" then
		return "Défausse 2 sorts de la main d'un adversaire."
	else
		return "Discard 2 spells from opponent's hand."
	end
end

function onCast()
end

function onResolve()
	selectTargetThenResolve("spell_in_hand", "opponent", "doResolve1")
end

function doResolve1(params)
	player, spell = split(params, " ")
	discardHandSpell(player, spell)
	selectTargetThenResolve("spell_in_hand", "opponent", "doResolve2")
end

function doResolve2(params)
	player, spell = split(params, " ")
	discardHandSpell(player, spell)
end
