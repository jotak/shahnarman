cost = {0,0,0,2}
icon = "archiviststrike"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Coup de l'archiviste"
	else
		return "Archivist strike"
	end
end

function getDescription()
	if language == "french" then
		return "Choisissez un sort de votre deck, et prenez-le en main."
	else
		return "Choose a spell from your deck and take it in hand."
	end
end

function onCast()
	-- nothing here, because the target selection can be done only during resolve phase (specific to "spell_in_deck" and "spell_in_hand" targets to avoid cheating)
end

function onResolve()
	selectTargetThenResolve("spell_in_deck", "owned", "doResolve")
end

-- this function receives playedId and spellId as parameter, in a space-separated string
function doResolve(params)
	player, spell = split(params, " ")
	drawSpell(player, spell)
end
